// @dependency-start
// responsibility Implements terminal popup tree sessions and bounded per-terminal logs.
// upstream implementation ../include/editor_proto/terminal_session.hpp declares terminal session API
// upstream design ../documents/mado_terminal_architecture.md terminal popup and log retention contract
// downstream implementation ../tests/cpp/prototype/terminal_session_test.cpp validates terminal behavior
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

TerminalLogBuffer::TerminalLogBuffer(TerminalLogConfig config) : config_(config) {}

void TerminalLogBuffer::append(TerminalLogKind kind, std::string message) {
  const std::size_t bytes = log_entry_size(message);
  entries_.push_back(TerminalLogEntry{.kind = kind, .message = std::move(message), .bytes = bytes});
  used_bytes_ += bytes;
  prune_old_entries();
}

void TerminalLogBuffer::clear() noexcept {
  entries_.clear();
  used_bytes_ = 0;
}

void TerminalLogBuffer::set_config(TerminalLogConfig config) {
  config_ = config;
  prune_old_entries();
}

const std::vector<TerminalLogEntry>& TerminalLogBuffer::entries() const noexcept { return entries_; }

std::size_t TerminalLogBuffer::max_bytes() const noexcept { return config_.max_bytes; }

std::size_t TerminalLogBuffer::used_bytes() const noexcept { return used_bytes_; }

void TerminalLogBuffer::prune_old_entries() {
  if (config_.max_bytes == 0U) {
    clear();
    return;
  }

  while (!entries_.empty() && used_bytes_ > config_.max_bytes) {
    used_bytes_ -= entries_.front().bytes;
    entries_.erase(entries_.begin());
  }
}

TerminalSessionTree::TerminalSessionTree(TerminalLogConfig log_config) : log_config_(log_config) {}

TerminalId TerminalSessionTree::create_terminal(TerminalLaunchRequest request) {
  TerminalSession session;
  session.id = next_id_++;
  session.parent_id = request.parent_id;
  session.title = std::move(request.title);
  session.root = std::move(request.root);
  session.launch_context = std::move(request.launch_context);
  session.popup_window = request.popup_window;
  session.log = TerminalLogBuffer(log_config_);

  const TerminalId id = session.id;
  sessions_.push_back(std::move(session));

  if (request.parent_id.has_value()) {
    TerminalSession* parent = find(*request.parent_id);
    if (parent != nullptr) parent->child_ids.push_back(id);
  }

  return id;
}

TerminalSession* TerminalSessionTree::find(TerminalId id) noexcept {
  const auto found = std::find_if(sessions_.begin(), sessions_.end(), [id](const TerminalSession& session) {
    return session.id == id;
  });
  if (found == sessions_.end()) return nullptr;
  return &*found;
}

const TerminalSession* TerminalSessionTree::find(TerminalId id) const noexcept {
  const auto found = std::find_if(sessions_.begin(), sessions_.end(), [id](const TerminalSession& session) {
    return session.id == id;
  });
  if (found == sessions_.end()) return nullptr;
  return &*found;
}

std::vector<TerminalId> TerminalSessionTree::roots() const {
  std::vector<TerminalId> ids;
  for (const TerminalSession& session : sessions_) {
    if (!session.parent_id.has_value()) ids.push_back(session.id);
  }
  return ids;
}

std::vector<TerminalId> TerminalSessionTree::children_of(TerminalId id) const {
  const TerminalSession* session = find(id);
  if (session == nullptr) return {};
  return session->child_ids;
}

std::size_t TerminalSessionTree::size() const noexcept { return sessions_.size(); }

void TerminalSessionTree::append_log(TerminalId id, TerminalLogKind kind, std::string message) {
  TerminalSession* session = find(id);
  if (session == nullptr) return;
  session->log.append(kind, std::move(message));
}

void TerminalSessionTree::set_log_config(TerminalLogConfig config) {
  log_config_ = config;
  for (TerminalSession& session : sessions_) {
    session.log.set_config(config);
  }
}

const char* terminal_log_kind_name(TerminalLogKind kind) noexcept {
  switch (kind) {
    case TerminalLogKind::Info:
      return "info";
    case TerminalLogKind::Input:
      return "input";
    case TerminalLogKind::Output:
      return "output";
    case TerminalLogKind::Error:
      return "error";
    case TerminalLogKind::Exit:
      return "exit";
  }
  return "unknown";
}

std::string make_child_terminal_title(std::string_view parent_title, std::string_view child_context) {
  std::ostringstream title;
  if (parent_title.empty()) {
    title << "Terminal";
  } else {
    title << parent_title;
  }
  if (!child_context.empty()) title << " / " << child_context;
  return title.str();
}

}  // namespace editor_proto
