// @dependency-start
// responsibility Defines terminal popup sessions with direct up/down links and bounded per-terminal log storage.
// upstream design ../../documents/mado_terminal_architecture.md terminal popup and log retention contract
// downstream implementation ../../src/terminal_session.cpp implements terminal session behavior
// downstream implementation ../../tests/cpp/prototype/terminal_session_test.cpp validates terminal links and log pruning
// @dependency-end
#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace editor_proto {

using TerminalId = std::size_t;

constexpr TerminalId kInvalidTerminalId = 0;

struct TerminalLogConfig {
  std::size_t max_bytes{64U * 1024U};
};

enum class TerminalLogKind {
  Info,
  Input,
  Output,
  Error,
  Exit,
};

struct TerminalLogEntry {
  TerminalLogKind kind{TerminalLogKind::Info};
  std::string message;
  std::size_t bytes{};
};

class TerminalLogBuffer {
 public:
  explicit TerminalLogBuffer(TerminalLogConfig config = {});

  void append(TerminalLogKind kind, std::string message);
  void clear() noexcept;
  void set_config(TerminalLogConfig config);

  [[nodiscard]] const std::vector<TerminalLogEntry>& entries() const noexcept;
  [[nodiscard]] std::size_t max_bytes() const noexcept;
  [[nodiscard]] std::size_t used_bytes() const noexcept;

 private:
  void prune_old_entries();

  TerminalLogConfig config_{};
  std::vector<TerminalLogEntry> entries_;
  std::size_t used_bytes_{};
};

struct TerminalSession {
  TerminalId id{kInvalidTerminalId};
  std::optional<TerminalId> parent_id;
  std::optional<TerminalId> child_id;
  std::string title;
  std::string root;
  std::string launch_context;
  bool popup_window{true};
  TerminalLogBuffer log;
};

struct TerminalLaunchRequest {
  std::optional<TerminalId> parent_id;
  std::string title;
  std::string root;
  std::string launch_context;
  bool popup_window{true};
};

class TerminalSessionChain {
 public:
  explicit TerminalSessionChain(TerminalLogConfig log_config = {});

  [[nodiscard]] TerminalId create_terminal(TerminalLaunchRequest request);
  [[nodiscard]] TerminalSession* find(TerminalId id) noexcept;
  [[nodiscard]] const TerminalSession* find(TerminalId id) const noexcept;
  [[nodiscard]] std::optional<TerminalId> parent_of(TerminalId id) const noexcept;
  [[nodiscard]] std::optional<TerminalId> child_of(TerminalId id) const noexcept;
  [[nodiscard]] std::size_t size() const noexcept;

  void append_log(TerminalId id, TerminalLogKind kind, std::string message);
  void set_log_config(TerminalLogConfig config);

 private:
  TerminalLogConfig log_config_{};
  TerminalId next_id_{1};
  std::vector<TerminalSession> sessions_;
};

[[nodiscard]] const char* terminal_log_kind_name(TerminalLogKind kind) noexcept;
[[nodiscard]] std::string make_child_terminal_title(std::string_view parent_title,
                                                    std::string_view child_context);

}  // namespace editor_proto
