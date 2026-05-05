#include "editor_proto/workspace_registry.hpp"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cout << "usage: editor_proto_cli <open-file|open-dir|scroll> <arg> [arg2]\n";
    return 0;
  }

  const std::string command = argv[1];
  editor_proto::WorkspaceRegistry registry;
  editor_proto::ScrollController scroll;

  if (command == "open-file") {
    const auto n1 = registry.open_file(argv[2]);
    const auto n2 = registry.open_file(argv[2]);
    std::cout << (n1.is_duplicate ? "dup" : "new") << "," << (n2.is_duplicate ? "dup" : "new") << "\n";
    return 0;
  }

  if (command == "open-dir") {
    const auto n1 = registry.add_directory_root(argv[2]);
    const auto n2 = registry.add_directory_root(argv[2]);
    std::cout << (n1.is_duplicate ? "dup" : "new") << "," << (n2.is_duplicate ? "dup" : "new") << "\n";
    return 0;
  }

  if (command == "scroll" && argc >= 4) {
    const double offset = std::stod(argv[2]);
    const double delta = std::stod(argv[3]);
    const auto r = scroll.middle_button_pan(offset, delta);
    std::cout << r.new_offset << "," << r.velocity << "\n";
    return 0;
  }

  std::cerr << "invalid command\n";
  return 1;
}
