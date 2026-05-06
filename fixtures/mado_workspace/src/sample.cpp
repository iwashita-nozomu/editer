/*
@dependency-start
responsibility Provides a small editable C++ file for Mado manual runtime checks.
upstream implementation ../../../src/editor_gui_proto.cpp opens and saves files from the test workspace
downstream design ../manual_checklist.md references this file in manual checks
@dependency-end
*/

#include <iostream>

int main() {
  std::cout << "Mado test workspace" << '\n';
  return 0;
}
