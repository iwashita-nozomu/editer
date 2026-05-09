// @dependency-start
// responsibility Validates popup session links, cascade close, focus restore, launch budget, and bounded logs.
// upstream implementation ../../../include/editor_proto/terminal_session.hpp declares popup session API
// upstream implementation ../../../src/terminal_session.cpp implements popup sessions
// downstream build ../../../CMakeLists.txt builds this executable as editor_proto_terminal_session_test
// @dependency-end
#include "editor_proto/terminal_session.hpp"

#include <cassert>
#include <string_view>

int main() {
  editor_proto::PopupSessionChain chain{editor_proto::PopupLogConfig{.max_bytes = 12U}};

  const editor_proto::PopupId local = chain.create_terminal(editor_proto::PopupLaunchRequest{
      .kind = editor_proto::PopupKind::Terminal,
      .parent_id = std::nullopt,
      .title = "local",
      .root = "/repo",
      .launch_context = "local shell",
      .launch_budget_ms = editor_proto::kDefaultPopupLaunchBudgetMs,
  });

  const editor_proto::PopupId remote = chain.create_terminal(editor_proto::PopupLaunchRequest{
      .kind = editor_proto::PopupKind::Terminal,
      .parent_id = local,
      .title = editor_proto::make_child_terminal_title("local", "ssh host"),
      .root = "/repo",
      .launch_context = "ssh host",
      .launch_budget_ms = editor_proto::kDefaultPopupLaunchBudgetMs,
  });

  const editor_proto::PopupId file = chain.create_file_popup(remote, "/repo", "src/main.cpp");

  assert(chain.size() == 3U);
  assert(chain.parent_of(local) == std::nullopt);
  assert(chain.child_of(local) == remote);
  assert(chain.parent_of(remote) == local);
  assert(chain.child_of(remote) == file);
  assert(chain.parent_of(file) == remote);
  assert(chain.child_of(file) == std::nullopt);

  const editor_proto::PopupSession* local_session = chain.find(local);
  const editor_proto::PopupSession* remote_session = chain.find(remote);
  const editor_proto::PopupSession* file_session = chain.find(file);
  assert(local_session != nullptr);
  assert(remote_session != nullptr);
  assert(file_session != nullptr);
  assert(local_session->kind == editor_proto::PopupKind::Terminal);
  assert(remote_session->kind == editor_proto::PopupKind::Terminal);
  assert(file_session->kind == editor_proto::PopupKind::File);
  assert(file_session->file_path == "src/main.cpp");
  assert(file_session->title == "main.cpp");
  assert(local_session->launch_budget_ms <= 100U);
  assert(remote_session->launch_budget_ms <= 100U);
  assert(file_session->launch_budget_ms <= 100U);

  chain.append_log(local, editor_proto::PopupLogKind::Output, "old-entry");
  chain.append_log(local, editor_proto::PopupLogKind::Error, "newer-entry");
  local_session = chain.find(local);
  assert(local_session != nullptr);
  assert(local_session->log.used_bytes() <= local_session->log.max_bytes());
  assert(local_session->log.entries().size() == 1U);
  assert(local_session->log.entries().front().message == "newer-entry");

  remote_session = chain.find(remote);
  assert(remote_session != nullptr);
  assert(remote_session->log.entries().empty());

  const editor_proto::PopupCloseResult file_close = chain.close_popup(file);
  assert(chain.is_closed(file));
  assert(!chain.is_closed(remote));
  assert(file_close.focus_restore_id == remote);

  const editor_proto::PopupCloseResult local_close = chain.close_terminal(local);
  assert(local_close.focus_restore_id == std::nullopt);
  assert(chain.is_closed(local));
  assert(chain.is_closed(remote));
  assert(chain.is_closed(file));

  assert(std::string_view(editor_proto::popup_kind_name(editor_proto::PopupKind::File)) == "file");
  assert(std::string_view(editor_proto::popup_log_kind_name(editor_proto::PopupLogKind::FileOpen)) ==
         "file-open");
  assert(editor_proto::make_file_popup_title("README.md") == "README.md");

  return 0;
}
