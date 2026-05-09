// @dependency-start
// responsibility Defines popup sessions with direct up/down links and bounded per-session log storage.
// upstream design ../../documents/mado_terminal_architecture.md popup and log retention contract
// downstream implementation ../../src/terminal_session.cpp implements popup session behavior
// downstream implementation ../../tests/cpp/prototype/terminal_session_test.cpp validates popup links, cascade close, focus restore, launch budget, and log pruning
// @dependency-end
#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace editor_proto {

using PopupId = std::size_t;
using TerminalId = PopupId;

constexpr PopupId kInvalidPopupId = 0;
constexpr TerminalId kInvalidTerminalId = kInvalidPopupId;
constexpr std::size_t kDefaultPopupLaunchBudgetMs = 100;

struct PopupLogConfig {
  std::size_t max_bytes{64U * 1024U};
};

using TerminalLogConfig = PopupLogConfig;

enum class PopupKind {
  Terminal,
  File,
};

enum class PopupLogKind {
  Info,
  Input,
  Output,
  Error,
  Exit,
  FileOpen,
};

using TerminalLogKind = PopupLogKind;

struct PopupLogEntry {
  PopupLogKind kind{PopupLogKind::Info};
  std::string message;
  std::size_t bytes{};
};

using TerminalLogEntry = PopupLogEntry;

class PopupLogBuffer {
 public:
  explicit PopupLogBuffer(PopupLogConfig config = {});

  void append(PopupLogKind kind, std::string message);
  void clear() noexcept;
  void set_config(PopupLogConfig config);

  [[nodiscard]] const std::vector<PopupLogEntry>& entries() const noexcept;
  [[nodiscard]] std::size_t max_bytes() const noexcept;
  [[nodiscard]] std::size_t used_bytes() const noexcept;

 private:
  void prune_old_entries();

  PopupLogConfig config_{};
  std::vector<PopupLogEntry> entries_;
  std::size_t used_bytes_{};
};

using TerminalLogBuffer = PopupLogBuffer;

struct PopupSession {
  PopupId id{kInvalidPopupId};
  PopupKind kind{PopupKind::Terminal};
  std::optional<PopupId> parent_id;
  std::optional<PopupId> child_id;
  std::string title;
  std::string root;
  std::string launch_context;
  std::string file_path;
  bool popup_window{true};
  bool closed{};
  std::size_t launch_budget_ms{kDefaultPopupLaunchBudgetMs};
  PopupLogBuffer log;
};

using TerminalSession = PopupSession;

struct PopupLaunchRequest {
  PopupKind kind{PopupKind::Terminal};
  std::optional<PopupId> parent_id;
  std::string title;
  std::string root;
  std::string launch_context;
  std::string file_path;
  bool popup_window{true};
  std::size_t launch_budget_ms{kDefaultPopupLaunchBudgetMs};
};

using TerminalLaunchRequest = PopupLaunchRequest;

struct PopupCloseResult {
  PopupId closed_root{kInvalidPopupId};
  std::optional<PopupId> focus_restore_id;
};

class PopupSessionChain {
 public:
  explicit PopupSessionChain(PopupLogConfig log_config = {});

  [[nodiscard]] PopupId create_popup(PopupLaunchRequest request);
  [[nodiscard]] PopupId create_terminal(PopupLaunchRequest request);
  [[nodiscard]] PopupId create_file_popup(std::optional<PopupId> parent_id, std::string root,
                                          std::string file_path);
  [[nodiscard]] PopupSession* find(PopupId id) noexcept;
  [[nodiscard]] const PopupSession* find(PopupId id) const noexcept;
  [[nodiscard]] std::optional<PopupId> parent_of(PopupId id) const noexcept;
  [[nodiscard]] std::optional<PopupId> child_of(PopupId id) const noexcept;
  [[nodiscard]] bool is_closed(PopupId id) const noexcept;
  [[nodiscard]] std::size_t size() const noexcept;

  void append_log(PopupId id, PopupLogKind kind, std::string message);
  [[nodiscard]] PopupCloseResult close_popup(PopupId id);
  [[nodiscard]] PopupCloseResult close_terminal(PopupId id);
  void set_log_config(PopupLogConfig config);

 private:
  void close_popup_chain(PopupId id);

  PopupLogConfig log_config_{};
  PopupId next_id_{1};
  std::vector<PopupSession> sessions_;
};

using TerminalSessionChain = PopupSessionChain;

[[nodiscard]] const char* popup_kind_name(PopupKind kind) noexcept;
[[nodiscard]] const char* popup_log_kind_name(PopupLogKind kind) noexcept;
[[nodiscard]] const char* terminal_log_kind_name(TerminalLogKind kind) noexcept;
[[nodiscard]] std::string make_child_popup_title(std::string_view parent_title,
                                                 std::string_view child_context);
[[nodiscard]] std::string make_child_terminal_title(std::string_view parent_title,
                                                    std::string_view child_context);
[[nodiscard]] std::string make_file_popup_title(std::string_view file_path);

}  // namespace editor_proto
