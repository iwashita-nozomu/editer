// @dependency-start
// responsibility Implements popup sessions with direct up/down links and bounded per-session logs.
// upstream implementation ../include/editor_proto/terminal_session.hpp declares popup session API
// upstream design ../documents/mado_terminal_architecture.md popup and log retention contract
// downstream implementation ../tests/cpp/prototype/terminal_session_test.cpp validates popup behavior
// @dependency-end
#include "editor_proto/terminal_session.hpp"

#include <algorithm>
#include <sstream>
#include <utility>

namespace editor_proto {

namespace {

std::size_t log_entry_size(std::string_view message) noexcept {
  return message.size();
}

}  // namespace

PopupLogBuffer::PopupLogBuffer(PopupLogConfig config) : config_(config) {}

void PopupLogBuffer::append(PopupLogKind kind, std::string message) {
  const std::size_t bytes = log_entry_size(message);
  entries_.push_back(PopupLogEntry{.kind = kind, .message = std::move(message), .bytes = bytes});
  used_bytes_ += bytes;
  prune_old_entries();
}

void PopupLogBuffer::clear() noexcept {
  entries_.clear();
  used_bytes_ = 0;
}

void PopupLogBuffer::set_config(PopupLogConfig config) {
  config_ = config;
  prune_old_entries();
}

const std::vector<PopupLogEntry>& PopupLogBuffer::entries() const noexcept { return entries_; }

std::size_t PopupLogBuffer::max_bytes() const noexcept { return config_.max_bytes; }

std::size_t PopupLogBuffer::used_bytes() const noexcept { return used_bytes_; }

void PopupLogBuffer::prune_old_entries() {
  if (config_.max_bytes == 0U) {
    clear();
    return;
  }

  while (!entries_.empty() && used_bytes_ > config_.max_bytes) {
    used_bytes_ -= entries_.front().bytes;
    entries_.erase(entries_.begin());
  }
}

PopupSessionChain::PopupSessionChain(PopupLogConfig log_config) : log_config_(log_config) {}

PopupId PopupSessionChain::create_popup(PopupLaunchRequest request) {
  PopupSession session;
  session.id = next_id_++;
  session.kind = request.kind;
  session.parent_id = request.parent_id;
  session.title = std::move(request.title);
  session.root = std::move(request.root);
  session.launch_context = std::move(request.launch_context);
  session.file_path = std::move(request.file_path);
  session.popup_window = request.popup_window;
  session.log = PopupLogBuffer(log_config_);

  const PopupId id = session.id;
  sessions_.push_back(std::move(session));

  if (request.parent_id.has_value()) {
    PopupSession* parent = find(*request.parent_id);
    if (parent != nullptr) parent->child_id = id;
  }

  return id;
}

PopupId PopupSessionChain::create_terminal(PopupLaunchRequest request) {
  request.kind = PopupKind::Terminal;
  return create_popup(std::move(request));
}

PopupId PopupSessionChain::create_file_popup(std::optional<PopupId> parent_id, std::string root,
                                             std::string file_path) {
  PopupLaunchRequest request;
  request.kind = PopupKind::File;
  request.parent_id = parent_id;
  request.title = make_file_popup_title(file_path);
  request.root = std::move(root);
  request.launch_context = "mado file";
  request.file_path = std::move(file_path);
  const PopupId id = create_popup(std::move(request));
  append_log(id, PopupLogKind::FileOpen, "opened file popup");
  return id;
}

PopupSession* PopupSessionChain::find(PopupId id) noexcept {
  const auto found = std::find_if(sessions_.begin(), sessions_.end(), [id](const PopupSession& session) {
    return session.id == id;
  });
  if (found == sessions_.end()) return nullptr;
  return &*found;
}

const PopupSession* PopupSessionChain::find(PopupId id) const noexcept {
  const auto found = std::find_if(sessions_.begin(), sessions_.end(), [id](const PopupSession& session) {
    return session.id == id;
  });
  if (found == sessions_.end()) return nullptr;
  return &*found;
}

std::optional<PopupId> PopupSessionChain::parent_of(PopupId id) const noexcept {
  const PopupSession* session = find(id);
  if (session == nullptr) return std::nullopt;
  return session->parent_id;
}

std::optional<PopupId> PopupSessionChain::child_of(PopupId id) const noexcept {
  const PopupSession* session = find(id);
  if (session == nullptr) return std::nullopt;
  return session->child_id;
}

bool PopupSessionChain::is_closed(PopupId id) const noexcept {
  const PopupSession* session = find(id);
  if (session == nullptr) return false;
  return session->closed;
}

std::size_t PopupSessionChain::size() const noexcept { return sessions_.size(); }

void PopupSessionChain::append_log(PopupId id, PopupLogKind kind, std::string message) {
  PopupSession* session = find(id);
  if (session == nullptr) return;
  session->log.append(kind, std::move(message));
}

void PopupSessionChain::close_popup(PopupId id) {
  PopupSession* session = find(id);
  if (session == nullptr || session->closed) return;

  session->closed = true;
  session->log.append(PopupLogKind::Exit, "closed popup");
  const std::optional<PopupId> child_id = session->child_id;
  if (child_id.has_value()) close_popup(*child_id);
}

void PopupSessionChain::close_terminal(PopupId id) { close_popup(id); }

void PopupSessionChain::set_log_config(PopupLogConfig config) {
  log_config_ = config;
  for (PopupSession& session : sessions_) {
    session.log.set_config(config);
  }
}

const char* popup_kind_name(PopupKind kind) noexcept {
  switch (kind) {
    case PopupKind::Terminal:
      return "terminal";
    case PopupKind::File:
      return "file";
  }
  return "unknown";
}

const char* popup_log_kind_name(PopupLogKind kind) noexcept {
  switch (kind) {
    case PopupLogKind::Info:
      return "info";
    case PopupLogKind::Input:
      return "input";
    case PopupLogKind::Output:
      return "output";
    case PopupLogKind::Error:
      return "error";
    case PopupLogKind::Exit:
      return "exit";
    case PopupLogKind::FileOpen:
      return "file-open";
  }
  return "unknown";
}

const char* terminal_log_kind_name(TerminalLogKind kind) noexcept {
  return popup_log_kind_name(kind);
}

std::string make_child_popup_title(std::string_view parent_title, std::string_view child_context) {
  std::ostringstream title;
  if (parent_title.empty()) {
    title << "Popup";
  } else {
    title << parent_title;
  }
  if (!child_context.empty()) title << " / " << child_context;
  return title.str();
}

std::string make_child_terminal_title(std::string_view parent_title, std::string_view child_context) {
  return make_child_popup_title(parent_title.empty() ? std::string_view("Terminal") : parent_title,
                                child_context);
}

std::string make_file_popup_title(std::string_view file_path) {
  if (file_path.empty()) return "File";
  const std::size_t slash = file_path.find_last_of("/\\");
  if (slash == std::string_view::npos) return std::string(file_path);
  return std::string(file_path.substr(slash + 1U));
}

}  // namespace editor_proto
