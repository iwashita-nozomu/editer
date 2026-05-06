// @dependency-start
// responsibility Provides the runnable CLI prototype for the editor core.
// upstream implementation ../include/editor_proto/workspace_registry.hpp workspace and scroll API
// upstream implementation ../CMakeLists.txt builds and tests this executable
// @dependency-end
#include "editor_proto/workspace_registry.hpp"

#include <cstddef>
#include <exception>
#include <iostream>
#include <string>

namespace {

std::string action_name(editor_proto::DuplicateAction action) {
  switch (action) {
    case editor_proto::DuplicateAction::FocusExisting:
      return "focus-existing";
    case editor_proto::DuplicateAction::OpenNewView:
      return "open-new-view";
    case editor_proto::DuplicateAction::RegisterAlias:
      return "register-alias";
  }
  return "unknown";
}

void print_notice(const std::string& label, const editor_proto::DuplicateNotice& notice) {
  std::cout << label << ":status=" << (notice.is_duplicate ? "duplicate" : "new")
            << " path=" << notice.canonical_path;
  if (!notice.actions.empty()) {
    std::cout << " actions=";
    for (std::size_t i = 0; i < notice.actions.size(); ++i) {
      if (i > 0) std::cout << "|";
      std::cout << action_name(notice.actions[i]);
    }
  }
  std::cout << "\n";
}

void print_usage() {
  std::cout << "usage: editor_proto_cli <demo|open-file|open-dir|scroll> [arg] [arg2]\n";
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    print_usage();
    return 0;
  }

  const std::string command = argv[1];
  editor_proto::WorkspaceRegistry registry;
  editor_proto::ScrollController scroll;

  if (command == "demo") {
    const auto file_first = registry.open_file("/Repo/src/main.cpp");
    const auto file_second = registry.open_file("\\repo\\src\\main.cpp");
    const auto root_first = registry.add_directory_root("/Work/ProjectA/");
    const auto root_second = registry.add_directory_root("/work/projecta");
    const auto pan = scroll.middle_button_pan(100.0, -10.0);

    print_notice("file:first", file_first);
    print_notice("file:second", file_second);
    print_notice("root:first", root_first);
    print_notice("root:second", root_second);
    std::cout << "scroll:pan offset=" << pan.new_offset << " velocity=" << pan.velocity << "\n";
    return 0;
  }

  if (command == "open-file") {
    if (argc < 3) {
      print_usage();
      return 1;
    }
    const auto n1 = registry.open_file(argv[2]);
    const auto n2 = registry.open_file(argv[2]);
    std::cout << (n1.is_duplicate ? "dup" : "new") << "," << (n2.is_duplicate ? "dup" : "new") << "\n";
    return 0;
  }

  if (command == "open-dir") {
    if (argc < 3) {
      print_usage();
      return 1;
    }
    const auto n1 = registry.add_directory_root(argv[2]);
    const auto n2 = registry.add_directory_root(argv[2]);
    std::cout << (n1.is_duplicate ? "dup" : "new") << "," << (n2.is_duplicate ? "dup" : "new") << "\n";
    return 0;
  }

  if (command == "scroll" && argc >= 4) {
    try {
      const double offset = std::stod(argv[2]);
      const double delta = std::stod(argv[3]);
      const auto r = scroll.middle_button_pan(offset, delta);
      std::cout << r.new_offset << "," << r.velocity << "\n";
    } catch (const std::exception& exc) {
      std::cerr << "invalid scroll argument: " << exc.what() << "\n";
      return 1;
    }
    return 0;
  }

  std::cerr << "invalid command\n";
  return 1;
}
