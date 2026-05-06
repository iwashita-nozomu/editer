// @dependency-start
// responsibility Provides the runnable native GUI prototype for the editor core.
// upstream implementation ../include/editor_proto/workspace_registry.hpp workspace and scroll API
// upstream implementation ../CMakeLists.txt builds and tests this executable
// downstream design README.md documents repository build entrypoints
// @dependency-end
#include "editor_proto/workspace_registry.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <sys/wait.h>
#include <thread>
#include <utility>
#include <vector>
#include <unistd.h>

#ifndef MADO_SOURCE_ROOT
#define MADO_SOURCE_ROOT "."
#endif

#ifndef MADO_BUILD_DIR
#define MADO_BUILD_DIR "build/cpp/dev"
#endif

#ifndef MADO_RUNTIME_BINARY
#define MADO_RUNTIME_BINARY ".state/cpp-install/mado/bin/mado"
#endif

namespace {

struct Display;

using Atom = unsigned long;
using Bool = int;
using Colormap = unsigned long;
using Drawable = unsigned long;
using Font = unsigned long;
using GC = void*;
using KeySym = unsigned long;
using Time = unsigned long;
using Window = unsigned long;

constexpr int X_CLIENT_DATA_BYTE_COUNT = 20;
constexpr int X_CLIENT_DATA_SHORT_COUNT = 10;
constexpr int X_CLIENT_DATA_LONG_COUNT = 5;
constexpr int X_EVENT_PAD_LONG_COUNT = 24;

constexpr int X_KEY_PRESS_EVENT = 2;
constexpr int X_BUTTON_PRESS_EVENT = 4;
constexpr int X_EXPOSE_EVENT = 12;
constexpr int X_CONFIGURE_NOTIFY_EVENT = 22;
constexpr int X_CLIENT_MESSAGE_EVENT = 33;
constexpr int X_REVERT_TO_PARENT = 2;
constexpr Time X_CURRENT_TIME = 0;

constexpr long X_KEY_PRESS_MASK_SHIFT = 0;
constexpr long X_BUTTON_PRESS_MASK_SHIFT = 2;
constexpr long X_EXPOSURE_MASK_SHIFT = 15;
constexpr long X_STRUCTURE_NOTIFY_MASK_SHIFT = 17;

constexpr unsigned int X_CONTROL_MASK_SHIFT = 2;
constexpr unsigned int X_BUTTON_PRIMARY = 1U;
constexpr unsigned int X_BUTTON_SCROLL_UP = 4U;
constexpr unsigned int X_BUTTON_SCROLL_DOWN = 5U;

constexpr KeySym X_KEY_BACK_SPACE = 0xff08;
constexpr KeySym X_KEY_RETURN = 0xff0d;
constexpr KeySym X_KEY_KEYPAD_ENTER = 0xff8d;
constexpr KeySym X_KEY_ESCAPE = 0xff1b;

constexpr double DEMO_SCROLL_OFFSET = 100.0;
constexpr double DEMO_POINTER_DELTA = -10.0;

constexpr int DEFAULT_WINDOW_X = 80;
constexpr int DEFAULT_WINDOW_Y = 80;
constexpr int DEFAULT_WINDOW_WIDTH = 1120;
constexpr int DEFAULT_WINDOW_HEIGHT = 720;
constexpr int EVENT_LOOP_SLEEP_MS = 12;
constexpr int KEY_LOOKUP_BUFFER_SIZE = 32;
constexpr unsigned char FIRST_PRINTABLE_ASCII = 32;
constexpr std::size_t MAX_PROJECT_FILES = 200;
constexpr std::uintmax_t MAX_FILE_BYTES = 262144;
constexpr int FLOATING_WINDOW_GAP = 24;
constexpr int FILE_PICKER_WIDTH = 440;
constexpr int FILE_PICKER_HEIGHT = 620;
constexpr int FILE_PICKER_ROW_START_Y = 112;
constexpr int FILE_PICKER_STATUS_BASELINE = 34;
constexpr int FILE_PICKER_CLOSE_X = 352;
constexpr int FILE_PICKER_CLOSE_WIDTH = 72;
constexpr int FILE_PICKER_ROOT_Y = 76;
constexpr int PROMPT_SHORTCUT_NUL = 0;
constexpr int COMMAND_EXEC_FAILURE_EXIT = 127;
constexpr KeySym X_KEY_AT = 0x0040;
constexpr KeySym X_KEY_UP = 0xff52;
constexpr KeySym X_KEY_DOWN = 0xff54;
constexpr int PROMPT_WINDOW_WIDTH = 560;
constexpr int PROMPT_WINDOW_HEIGHT = 360;
constexpr int PROMPT_ROOT_Y = 60;
constexpr int PROMPT_SHELL_Y = 78;
constexpr int PROMPT_INPUT_Y = 94;
constexpr int PROMPT_INPUT_HEIGHT = 28;
constexpr int PROMPT_OUTPUT_Y = 134;
constexpr int PROMPT_RUN_WIDTH = 64;
constexpr int PROMPT_CLOSE_WIDTH = 72;
constexpr int PROMPT_BUTTON_GAP = 8;
constexpr int PROMPT_BUTTON_RIGHT_PAD = 16;
constexpr int PROMPT_OUTPUT_BOTTOM_PAD = 16;
constexpr int PROMPT_MIN_CONTENT_WIDTH = 96;
constexpr int PROMPT_SCROLL_STEP_LINES = 3;
constexpr int COMMAND_READ_BUFFER_SIZE = 1024;
constexpr int PIPE_READ_END = 0;
constexpr int PIPE_WRITE_END = 1;
constexpr int COMMAND_EXIT_STATUS_OK = 0;
constexpr int COMMAND_SIGNAL_STATUS_BASE = 128;
constexpr std::size_t MAX_PROMPT_LINES = 64;
constexpr int DEFAULT_FONT_SIZE = 14;
constexpr int MIN_FONT_SIZE = 10;
constexpr int MAX_FONT_SIZE = 24;
constexpr int FONT_SIZE_SMALL_MAX = 11;
constexpr int FONT_SIZE_DEFAULT_MAX = 14;
constexpr int FONT_SIZE_MEDIUM_MAX = 16;
constexpr int FONT_SIZE_LARGE_MAX = 20;
constexpr int FONT_LINE_HEIGHT_PAD = 4;
constexpr int FONT_CHAR_WIDTH_NUMERATOR = 4;
constexpr int FONT_CHAR_WIDTH_DENOMINATOR = 7;
constexpr int MIN_CHAR_WIDTH = 6;
constexpr int X_FONT_QUERY_LIMIT = 1;
constexpr int NO_PROCESS_STATUS = -1;
constexpr int CTRL_PREFIX_LENGTH = 5;
constexpr char YAML_SECTION_SUFFIX = ':';
constexpr std::string_view DEFAULT_APP_NAME = "Mado";
constexpr std::string_view DEFAULT_CONFIG_FILE = "mado.yaml";
constexpr std::string_view UPDATE_TARGET = "mado";
constexpr std::string_view BUILD_CACHE_FILE = "CMakeCache.txt";
constexpr std::string_view BUILD_BINARY_DIR = "bin";
constexpr std::string_view UPDATE_TEMP_SUFFIX = ".tmp";
constexpr std::string_view TEST_WORKSPACE_DIR = "fixtures/mado_workspace";

namespace ui {

constexpr int TOPBAR_HEIGHT = 44;
constexpr int STATUS_HEIGHT = 24;
constexpr int INSPECTOR_COMPACT_THRESHOLD = 980;
constexpr int INSPECTOR_COMPACT_WIDTH = 250;
constexpr int INSPECTOR_DEFAULT_WIDTH = 310;
constexpr int BORDER_OFFSET = 1;
constexpr int TITLE_X = 10;
constexpr int TITLE_BASELINE = 26;
constexpr int HEADER_BOTTOM = 38;
constexpr int SMALL_GAP = 6;
constexpr int TOOLBAR_TITLE_X = 12;
constexpr int TOOLBAR_BASELINE = 27;
constexpr int TOOLBAR_BUTTON_START_X = 192;
constexpr int TOOLBAR_BUTTON_TOP = 8;
constexpr int TOOLBAR_BUTTON_HEIGHT = 28;
constexpr int FILES_BUTTON_WIDTH = 70;
constexpr int STANDARD_BUTTON_WIDTH = 64;
constexpr int DEMO_BUTTON_WIDTH = 72;
constexpr int PROMPT_BUTTON_WIDTH = 72;
constexpr int CLOSE_BUTTON_WIDTH = 72;
constexpr int WORKSPACE_LABEL_RIGHT_PAD = 210;
constexpr int ROOT_LABEL_X = 12;
constexpr int ROOT_LABEL_BASELINE = 66;
constexpr int FILE_ROW_START_Y = 90;
constexpr int ROW_X_INSET = 8;
constexpr int ROW_WIDTH_INSET = 16;
constexpr int ROW_HEIGHT = 26;
constexpr int ROW_STEP = 30;
constexpr int FILE_ICON_X = 18;
constexpr int FILE_LABEL_X = 42;
constexpr int FILE_ROW_BASELINE = 18;
constexpr int FILE_LABEL_WIDTH_PAD = 64;
constexpr int TAB_HEADER_HEIGHT = 38;
constexpr int TAB_HEADER_BOTTOM = 37;
constexpr int TAB_START_X = 8;
constexpr int TAB_TOP = 7;
constexpr int TAB_WIDTH = 148;
constexpr int TAB_HEIGHT = 26;
constexpr int TAB_LABEL_X = 8;
constexpr int TAB_BASELINE = 18;
constexpr int TAB_TEXT_WIDTH = 132;
constexpr int TAB_STEP = 154;
constexpr int EDITOR_FIRST_LINE_Y = 64;
constexpr int EDITOR_LINE_NUMBER_X = 16;
constexpr int EDITOR_TEXT_X = 56;
constexpr int EDITOR_TEXT_WIDTH_PAD = 76;
constexpr int TEXT_LINE_HEIGHT = 17;
constexpr int EDITOR_BOTTOM_PAD = 74;
constexpr int NOTICE_CARD_X = 10;
constexpr int NOTICE_CARD_Y = 54;
constexpr int NOTICE_CARD_WIDTH_PAD = 20;
constexpr int NOTICE_CARD_HEIGHT = 118;
constexpr int NOTICE_TITLE_BASELINE = 25;
constexpr int NOTICE_PATH_BASELINE = 50;
constexpr int NOTICE_ACTIONS_BASELINE = 76;
constexpr int CORE_LABEL_BASELINE = 202;
constexpr int CORE_DETAIL_BASELINE = 226;
constexpr int STATUS_TEXT_X = 10;
constexpr int STATUS_BASELINE = 17;
constexpr int STATUS_TEXT_WIDTH_PAD = 310;
constexpr int STATUS_HINT_RIGHT_PAD = 300;
constexpr int BUTTON_LABEL_X = 10;
constexpr int BUTTON_LABEL_BASELINE = 19;
constexpr int ELLIPSIS_WIDTH = 3;

}  // namespace ui

struct XColor {
  unsigned long pixel;
  unsigned short red;
  unsigned short green;
  unsigned short blue;
  char flags;
  char pad;
};

struct XAnyEvent {
  int type;
  unsigned long serial;
  Bool send_event;
  Display* display;
  Window window;
};

struct XKeyEvent {
  int type;
  unsigned long serial;
  Bool send_event;
  Display* display;
  Window window;
  Window root;
  Window subwindow;
  Time time;
  int x;
  int y;
  int x_root;
  int y_root;
  unsigned int state;
  unsigned int keycode;
  Bool same_screen;
};

struct XButtonEvent {
  int type;
  unsigned long serial;
  Bool send_event;
  Display* display;
  Window window;
  Window root;
  Window subwindow;
  Time time;
  int x;
  int y;
  int x_root;
  int y_root;
  unsigned int state;
  unsigned int button;
  Bool same_screen;
};

struct XConfigureEvent {
  int type;
  unsigned long serial;
  Bool send_event;
  Display* display;
  Window event;
  Window window;
  int x;
  int y;
  int width;
  int height;
  int border_width;
  Window above;
  Bool override_redirect;
};

union XClientData {
  char b[X_CLIENT_DATA_BYTE_COUNT];
  short s[X_CLIENT_DATA_SHORT_COUNT];
  long l[X_CLIENT_DATA_LONG_COUNT];
};

struct XClientMessageEvent {
  int type;
  unsigned long serial;
  Bool send_event;
  Display* display;
  Window window;
  Atom message_type;
  int format;
  XClientData data;
};

union XEvent {
  int type;
  XAnyEvent xany;
  XKeyEvent xkey;
  XButtonEvent xbutton;
  XConfigureEvent xconfigure;
  XClientMessageEvent xclient;
  long pad[X_EVENT_PAD_LONG_COUNT];
};

constexpr int kKeyPress = X_KEY_PRESS_EVENT;
constexpr int kButtonPress = X_BUTTON_PRESS_EVENT;
constexpr int kExpose = X_EXPOSE_EVENT;
constexpr int kConfigureNotify = X_CONFIGURE_NOTIFY_EVENT;
constexpr int kClientMessage = X_CLIENT_MESSAGE_EVENT;

constexpr long kKeyPressMask = 1L << X_KEY_PRESS_MASK_SHIFT;
constexpr long kButtonPressMask = 1L << X_BUTTON_PRESS_MASK_SHIFT;
constexpr long kExposureMask = 1L << X_EXPOSURE_MASK_SHIFT;
constexpr long kStructureNotifyMask = 1L << X_STRUCTURE_NOTIFY_MASK_SHIFT;

constexpr unsigned int kControlMask = 1U << X_CONTROL_MASK_SHIFT;
constexpr unsigned int kButton1 = X_BUTTON_PRIMARY;

constexpr KeySym kBackSpace = X_KEY_BACK_SPACE;
constexpr KeySym kReturn = X_KEY_RETURN;
constexpr KeySym kKeypadEnter = X_KEY_KEYPAD_ENTER;
constexpr KeySym kEscape = X_KEY_ESCAPE;

template <typename Fn>
Fn load_symbol(void* library, const char* name) {
  dlerror();
  void* symbol = dlsym(library, name);
  const char* error = dlerror();
  if (error != nullptr || symbol == nullptr) {
    throw std::runtime_error(std::string("missing X11 symbol: ") + name);
  }
  return reinterpret_cast<Fn>(symbol);
}

class X11 {
 public:
  X11() : library_(dlopen("libX11.so.6", RTLD_LAZY | RTLD_LOCAL)) {
    if (library_ == nullptr) {
      throw std::runtime_error(std::string("failed to load libX11.so.6: ") + dlerror());
    }

    open_display = load_symbol<Display* (*)(const char*)>(library_, "XOpenDisplay");
    default_screen = load_symbol<int (*)(Display*)>(library_, "XDefaultScreen");
    root_window = load_symbol<Window (*)(Display*, int)>(library_, "XRootWindow");
    black_pixel = load_symbol<unsigned long (*)(Display*, int)>(library_, "XBlackPixel");
    white_pixel = load_symbol<unsigned long (*)(Display*, int)>(library_, "XWhitePixel");
    create_simple_window = load_symbol<Window (*)(Display*, Window, int, int, unsigned int,
                                                   unsigned int, unsigned int, unsigned long,
                                                   unsigned long)>(library_, "XCreateSimpleWindow");
    store_name = load_symbol<int (*)(Display*, Window, const char*)>(library_, "XStoreName");
    select_input = load_symbol<int (*)(Display*, Window, long)>(library_, "XSelectInput");
    map_window = load_symbol<int (*)(Display*, Window)>(library_, "XMapWindow");
    set_input_focus = load_symbol<int (*)(Display*, Window, int, Time)>(library_,
                                                                        "XSetInputFocus");
    list_fonts = load_symbol<char** (*)(Display*, const char*, int, int*)>(library_, "XListFonts");
    free_font_names = load_symbol<int (*)(char**)>(library_, "XFreeFontNames");
    load_font = load_symbol<Font (*)(Display*, const char*)>(library_, "XLoadFont");
    set_font = load_symbol<int (*)(Display*, GC, Font)>(library_, "XSetFont");
    unload_font = load_symbol<int (*)(Display*, Font)>(library_, "XUnloadFont");
    create_gc = load_symbol<GC (*)(Display*, Drawable, unsigned long, void*)>(library_, "XCreateGC");
    set_foreground = load_symbol<int (*)(Display*, GC, unsigned long)>(library_, "XSetForeground");
    fill_rectangle = load_symbol<int (*)(Display*, Drawable, GC, int, int, unsigned int,
                                          unsigned int)>(library_, "XFillRectangle");
    draw_string = load_symbol<int (*)(Display*, Drawable, GC, int, int, const char*, int)>(
        library_, "XDrawString");
    draw_line = load_symbol<int (*)(Display*, Drawable, GC, int, int, int, int)>(library_,
                                                                                 "XDrawLine");
    flush = load_symbol<int (*)(Display*)>(library_, "XFlush");
    raise_window = load_symbol<int (*)(Display*, Window)>(library_, "XRaiseWindow");
    intern_atom = load_symbol<Atom (*)(Display*, const char*, Bool)>(library_, "XInternAtom");
    set_wm_protocols = load_symbol<int (*)(Display*, Window, Atom*, int)>(library_,
                                                                           "XSetWMProtocols");
    pending = load_symbol<int (*)(Display*)>(library_, "XPending");
    next_event = load_symbol<int (*)(Display*, XEvent*)>(library_, "XNextEvent");
    destroy_window = load_symbol<int (*)(Display*, Window)>(library_, "XDestroyWindow");
    close_display = load_symbol<int (*)(Display*)>(library_, "XCloseDisplay");
    free_gc = load_symbol<int (*)(Display*, GC)>(library_, "XFreeGC");
    default_colormap = load_symbol<Colormap (*)(Display*, int)>(library_, "XDefaultColormap");
    parse_color = load_symbol<int (*)(Display*, Colormap, const char*, XColor*)>(library_,
                                                                                 "XParseColor");
    alloc_color = load_symbol<int (*)(Display*, Colormap, XColor*)>(library_, "XAllocColor");
    lookup_string = load_symbol<int (*)(XKeyEvent*, char*, int, KeySym*, void*)>(library_,
                                                                                 "XLookupString");
  }

  X11(const X11&) = delete;
  X11& operator=(const X11&) = delete;

  ~X11() {
    if (library_ != nullptr) {
      dlclose(library_);
    }
  }

  Display* (*open_display)(const char*){};
  int (*default_screen)(Display*){};
  Window (*root_window)(Display*, int){};
  unsigned long (*black_pixel)(Display*, int){};
  unsigned long (*white_pixel)(Display*, int){};
  Window (*create_simple_window)(Display*, Window, int, int, unsigned int, unsigned int,
                                 unsigned int, unsigned long, unsigned long){};
  int (*store_name)(Display*, Window, const char*){};
  int (*select_input)(Display*, Window, long){};
  int (*map_window)(Display*, Window){};
  int (*set_input_focus)(Display*, Window, int, Time){};
  char** (*list_fonts)(Display*, const char*, int, int*){};
  int (*free_font_names)(char**){};
  Font (*load_font)(Display*, const char*){};
  int (*set_font)(Display*, GC, Font){};
  int (*unload_font)(Display*, Font){};
  GC (*create_gc)(Display*, Drawable, unsigned long, void*){};
  int (*set_foreground)(Display*, GC, unsigned long){};
  int (*fill_rectangle)(Display*, Drawable, GC, int, int, unsigned int, unsigned int){};
  int (*draw_string)(Display*, Drawable, GC, int, int, const char*, int){};
  int (*draw_line)(Display*, Drawable, GC, int, int, int, int){};
  int (*flush)(Display*){};
  int (*raise_window)(Display*, Window){};
  Atom (*intern_atom)(Display*, const char*, Bool){};
  int (*set_wm_protocols)(Display*, Window, Atom*, int){};
  int (*pending)(Display*){};
  int (*next_event)(Display*, XEvent*){};
  int (*destroy_window)(Display*, Window){};
  int (*close_display)(Display*){};
  int (*free_gc)(Display*, GC){};
  Colormap (*default_colormap)(Display*, int){};
  int (*parse_color)(Display*, Colormap, const char*, XColor*){};
  int (*alloc_color)(Display*, Colormap, XColor*){};
  int (*lookup_string)(XKeyEvent*, char*, int, KeySym*, void*){};

 private:
  void* library_{};
};

struct Rect {
  int x;
  int y;
  int width;
  int height;
};

bool contains(Rect rect, int x, int y) {
  return x >= rect.x && y >= rect.y && x < rect.x + rect.width && y < rect.y + rect.height;
}

enum class HitAction {
  OpenFilesWindow,
  OpenPromptWindow,
  SplitView,
  SaveFile,
  RunCore,
  CloseWindow,
  ClosePicker,
  ClosePrompt,
  RunPrompt,
  SelectFile,
};

enum class FocusPane {
  Editor,
  Notice,
  Prompt,
};

struct HitRegion {
  Rect rect;
  HitAction action;
  int index{-1};
};

struct EditorFile {
  std::filesystem::path path;
  std::string label;
  std::string content;
  std::size_t cursor{};
  bool dirty{};
};

const char* action_name(editor_proto::DuplicateAction action) {
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

std::string actions_text(const std::vector<editor_proto::DuplicateAction>& actions) {
  if (actions.empty()) return "no conflict";

  std::ostringstream out;
  for (std::size_t i = 0; i < actions.size(); ++i) {
    if (i > 0) out << " | ";
    out << action_name(actions[i]);
  }
  return out.str();
}

std::string notice_text(std::string_view kind, const editor_proto::DuplicateNotice& notice) {
  std::ostringstream out;
  out << kind << ':' << (notice.is_duplicate ? "duplicate" : "new") << ' '
      << notice.canonical_path;
  const std::string actions = actions_text(notice.actions);
  if (actions != "no conflict") out << " actions=" << actions;
  return out.str();
}

std::string demo_text() {
  editor_proto::WorkspaceRegistry registry;
  editor_proto::ScrollController scroll;

  const auto file_first = registry.open_file("/Repo/src/main.cpp");
  const auto file_second = registry.open_file("\\repo\\src\\main.cpp");
  const auto root_first = registry.add_directory_root("/Work/ProjectA/");
  const auto root_second = registry.add_directory_root("/work/projecta");
  const auto pan = scroll.middle_button_pan(DEMO_SCROLL_OFFSET, DEMO_POINTER_DELTA);

  std::ostringstream out;
  out << notice_text("file:first", file_first) << '\n'
      << notice_text("file:second", file_second) << '\n'
      << notice_text("root:first", root_first) << '\n'
      << notice_text("root:second", root_second) << '\n'
      << "scroll:pan offset=" << pan.new_offset << " velocity=" << pan.velocity;
  return out.str();
}

struct KeyBinding {
  bool ctrl{true};
  char key{};
  std::string label;
};

struct KeyBindings {
  KeyBinding open_files{true, 'o', "Ctrl+O"};
  KeyBinding save{true, 's', "Ctrl+S"};
  KeyBinding prompt{true, '@', "Ctrl+@"};
  KeyBinding demo{true, 'd', "Ctrl+D"};
  KeyBinding quit{true, 'q', "Ctrl+Q"};
  KeyBinding focus_editor{true, '1', "Ctrl+1"};
  KeyBinding focus_notice{true, '2', "Ctrl+2"};
};

struct AppConfig {
  std::string app_name{std::string(DEFAULT_APP_NAME)};
  int font_size{DEFAULT_FONT_SIZE};
  KeyBindings keys{};
  std::filesystem::path source_root{std::filesystem::path(MADO_SOURCE_ROOT)};
  std::filesystem::path build_dir{std::filesystem::path(MADO_BUILD_DIR)};
  std::filesystem::path runtime_binary{std::filesystem::path(MADO_RUNTIME_BINARY)};
};

std::string trim(std::string value) {
  const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
    return std::isspace(ch) != 0;
  });
  const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
                      return std::isspace(ch) != 0;
                    }).base();
  if (first >= last) return {};
  return std::string(first, last);
}

std::string strip_comment(std::string value) {
  const std::size_t marker = value.find('#');
  if (marker != std::string::npos) value.erase(marker);
  return value;
}

std::string unquote(std::string value) {
  value = trim(std::move(value));
  if (value.size() < 2U) return value;
  const char first = value.front();
  const char last = value.back();
  if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
    return value.substr(1U, value.size() - 2U);
  }
  return value;
}

std::string uppercase(std::string value) {
  for (char& ch : value) {
    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
  }
  return value;
}

int parse_int(std::string_view value, int fallback) {
  try {
    return std::stoi(std::string(value));
  } catch (const std::exception&) {
    return fallback;
  }
}

KeyBinding parse_key_binding(std::string value, KeyBinding fallback) {
  value = unquote(std::move(value));
  std::string compact;
  for (char ch : value) {
    if (std::isspace(static_cast<unsigned char>(ch)) == 0) compact.push_back(ch);
  }
  const std::string upper = uppercase(compact);
  if (upper.rfind("CTRL+", 0U) != 0U || compact.size() <= CTRL_PREFIX_LENGTH) return fallback;
  fallback.ctrl = true;
  fallback.key = compact[CTRL_PREFIX_LENGTH];
  fallback.label = "Ctrl+";
  fallback.label.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(fallback.key))));
  if (fallback.key == '@') fallback.label = "Ctrl+@";
  return fallback;
}

void apply_config_value(AppConfig& config, const std::string& section, const std::string& key,
                        const std::string& raw_value) {
  const std::string value = unquote(raw_value);
  if (section == "app" && key == "name") {
    if (!value.empty()) config.app_name = value;
  } else if (section == "font" && key == "size") {
    config.font_size = std::clamp(parse_int(value, config.font_size), MIN_FONT_SIZE, MAX_FONT_SIZE);
  } else if (section == "keybindings" && key == "open_files") {
    config.keys.open_files = parse_key_binding(value, config.keys.open_files);
  } else if (section == "keybindings" && key == "save") {
    config.keys.save = parse_key_binding(value, config.keys.save);
  } else if (section == "keybindings" && key == "prompt") {
    config.keys.prompt = parse_key_binding(value, config.keys.prompt);
  } else if (section == "keybindings" && key == "demo") {
    config.keys.demo = parse_key_binding(value, config.keys.demo);
  } else if (section == "keybindings" && key == "quit") {
    config.keys.quit = parse_key_binding(value, config.keys.quit);
  } else if (section == "keybindings" && key == "focus_editor") {
    config.keys.focus_editor = parse_key_binding(value, config.keys.focus_editor);
  } else if (section == "keybindings" && key == "focus_notice") {
    config.keys.focus_notice = parse_key_binding(value, config.keys.focus_notice);
  } else if (section == "update" && key == "source_root") {
    config.source_root = value;
  } else if (section == "update" && key == "build_dir") {
    config.build_dir = value;
  } else if (section == "update" && key == "runtime_binary") {
    config.runtime_binary = value;
  }
}

std::filesystem::path default_config_path() {
  std::filesystem::path cwd_config{std::string(DEFAULT_CONFIG_FILE)};
  if (std::filesystem::exists(cwd_config)) return cwd_config;
  return std::filesystem::path(MADO_SOURCE_ROOT) / std::string(DEFAULT_CONFIG_FILE);
}

AppConfig load_config(const std::filesystem::path& path) {
  AppConfig config;
  std::ifstream in(path);
  if (!in) return config;

  std::string section;
  std::string line;
  while (std::getline(in, line)) {
    line = strip_comment(std::move(line));
    if (trim(line).empty()) continue;
    const bool child = !line.empty() && std::isspace(static_cast<unsigned char>(line.front())) != 0;
    line = trim(std::move(line));
    const std::size_t colon = line.find(YAML_SECTION_SUFFIX);
    if (colon == std::string::npos) continue;
    std::string key = trim(line.substr(0U, colon));
    std::string value = trim(line.substr(colon + 1U));
    if (!child && value.empty()) {
      section = std::move(key);
      continue;
    }
    apply_config_value(config, child ? section : std::string{}, key, value);
  }
  const std::filesystem::path config_base = std::filesystem::absolute(path).parent_path();
  if (!config.source_root.is_absolute()) config.source_root = config_base / config.source_root;
  return config;
}

std::filesystem::path resolve_from_source(const std::filesystem::path& source_root,
                                          const std::filesystem::path& path) {
  if (path.is_absolute()) return path;
  return source_root / path;
}

int run_program(const std::vector<std::string>& args) {
  if (args.empty()) return NO_PROCESS_STATUS;
  const pid_t pid = fork();
  if (pid < 0) return NO_PROCESS_STATUS;
  if (pid == 0) {
    std::vector<char*> argv;
    argv.reserve(args.size() + 1U);
    for (const std::string& arg : args) argv.push_back(const_cast<char*>(arg.c_str()));
    argv.push_back(nullptr);
    execvp(argv.front(), argv.data());
    _exit(COMMAND_EXEC_FAILURE_EXIT);
  }
  int status = COMMAND_EXIT_STATUS_OK;
  waitpid(pid, &status, 0);
  if (WIFEXITED(status)) return WEXITSTATUS(status);
  if (WIFSIGNALED(status)) return COMMAND_SIGNAL_STATUS_BASE + WTERMSIG(status);
  return NO_PROCESS_STATUS;
}

int run_update(const AppConfig& config, bool restart) {
  const std::filesystem::path source_root = std::filesystem::absolute(config.source_root);
  const std::filesystem::path build_dir = resolve_from_source(source_root, config.build_dir);
  const std::filesystem::path runtime_binary =
      resolve_from_source(source_root, config.runtime_binary);
  const std::filesystem::path build_binary =
      build_dir / std::string(BUILD_BINARY_DIR) / std::string(UPDATE_TARGET);

  std::error_code ec;
  std::filesystem::create_directories(build_dir, ec);
  if (ec) {
    std::cerr << "update: failed to create build dir " << build_dir << "\n";
    return 1;
  }
  if (!std::filesystem::exists(build_dir / std::string(BUILD_CACHE_FILE))) {
    const int configure_status =
        run_program({"cmake", "-S", source_root.string(), "-B", build_dir.string(), "-G", "Ninja"});
    if (configure_status != COMMAND_EXIT_STATUS_OK) return configure_status;
  }
  const int build_status =
      run_program({"cmake", "--build", build_dir.string(), "--target", std::string(UPDATE_TARGET)});
  if (build_status != COMMAND_EXIT_STATUS_OK) return build_status;

  std::filesystem::create_directories(runtime_binary.parent_path(), ec);
  if (ec) {
    std::cerr << "update: failed to create runtime dir " << runtime_binary.parent_path() << "\n";
    return 1;
  }
  const std::filesystem::path temp_binary = runtime_binary.string() + std::string(UPDATE_TEMP_SUFFIX);
  std::filesystem::copy_file(build_binary, temp_binary, std::filesystem::copy_options::overwrite_existing,
                             ec);
  if (ec) {
    std::cerr << "update: failed to copy " << build_binary << " to " << temp_binary << "\n";
    return 1;
  }
  std::filesystem::permissions(temp_binary, std::filesystem::perms::owner_exec |
                                                std::filesystem::perms::group_exec |
                                                std::filesystem::perms::others_exec,
                               std::filesystem::perm_options::add, ec);
  std::filesystem::rename(temp_binary, runtime_binary, ec);
  if (ec) {
    std::cerr << "update: failed to replace " << runtime_binary << "\n";
    return 1;
  }
  std::cout << "updated " << runtime_binary << "\n";
  if (!restart) return COMMAND_EXIT_STATUS_OK;
  execl(runtime_binary.c_str(), runtime_binary.c_str(), nullptr);
  std::cerr << "update: restart failed " << runtime_binary << "\n";
  return 1;
}

class NativeEditorApp {
 public:
  NativeEditorApp(AppConfig config, std::filesystem::path root_path)
      : config_(std::move(config)), root_path_(std::move(root_path)) {
    refresh_project_files();
  }

  void run() {
    display_ = x11_.open_display(nullptr);
    if (display_ == nullptr) {
      const char* display_env = std::getenv("DISPLAY");
      std::ostringstream out;
      out << "failed to open X display";
      if (display_env != nullptr) out << " (" << display_env << ")";
      throw std::runtime_error(out.str());
    }

    screen_ = x11_.default_screen(display_);
    const Window root = x11_.root_window(display_, screen_);
    black_ = x11_.black_pixel(display_, screen_);
    white_ = x11_.white_pixel(display_, screen_);
    colormap_ = x11_.default_colormap(display_, screen_);
    load_palette();

    window_ = x11_.create_simple_window(display_, root, DEFAULT_WINDOW_X, DEFAULT_WINDOW_Y,
                                        static_cast<unsigned int>(width_),
                                        static_cast<unsigned int>(height_), 1, palette_.border,
                                        palette_.background);
    if (window_ == 0) {
      throw std::runtime_error("failed to create X11 window");
    }

    x11_.store_name(display_, window_, config_.app_name.c_str());
    x11_.select_input(display_, window_,
                      kExposureMask | kStructureNotifyMask | kKeyPressMask | kButtonPressMask);
    wm_delete_ = x11_.intern_atom(display_, "WM_DELETE_WINDOW", 0);
    x11_.set_wm_protocols(display_, window_, &wm_delete_, 1);
    gc_ = x11_.create_gc(display_, window_, 0, nullptr);
    load_font();
    x11_.map_window(display_, window_);

    if (!files_.empty()) {
      open_file(0, true);
    } else {
      status_ = "no editable files under " + root_label();
    }
    log("native X11 window ready");
    std::cout << config_.app_name << ": native X11 window opened\n";
    std::cout << "Use " << config_.keys.open_files.label << " to open files, "
              << config_.keys.prompt.label << " for the floating prompt.\n" << std::flush;

    bool running = true;
    while (running) {
      while (x11_.pending(display_) > 0) {
        XEvent event{};
        x11_.next_event(display_, &event);
        running = handle_event(event);
        if (!running) break;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(EVENT_LOOP_SLEEP_MS));
    }
  }

  ~NativeEditorApp() {
    if (display_ == nullptr) return;
    close_file_picker();
    close_prompt_window();
    if (gc_ != nullptr) {
      if (font_ != 0) {
        x11_.unload_font(display_, font_);
        font_ = 0;
      }
      x11_.free_gc(display_, gc_);
      gc_ = nullptr;
    }
    if (window_ != 0) {
      x11_.destroy_window(display_, window_);
      window_ = 0;
    }
    x11_.close_display(display_);
    display_ = nullptr;
  }

 private:
  struct Palette {
    unsigned long background{};
    unsigned long topbar{};
    unsigned long panel{};
    unsigned long editor{};
    unsigned long border{};
    unsigned long text{};
    unsigned long muted{};
    unsigned long accent{};
    unsigned long ok{};
    unsigned long warn{};
  };

  int line_height() const { return config_.font_size + FONT_LINE_HEIGHT_PAD; }

  int char_width() const {
    return std::max(MIN_CHAR_WIDTH,
                    (config_.font_size * FONT_CHAR_WIDTH_NUMERATOR) / FONT_CHAR_WIDTH_DENOMINATOR);
  }

  std::string font_pattern() const {
    const int size = config_.font_size;
    if (size <= FONT_SIZE_SMALL_MAX) return "6x10";
    if (size <= FONT_SIZE_DEFAULT_MAX) return "7x14";
    if (size <= FONT_SIZE_MEDIUM_MAX) return "9x15";
    if (size <= FONT_SIZE_LARGE_MAX) return "10x20";
    return "12x24";
  }

  void load_font() {
    const std::string pattern = font_pattern();
    int font_count = 0;
    char** names = x11_.list_fonts(display_, pattern.c_str(), X_FONT_QUERY_LIMIT, &font_count);
    if (names == nullptr || font_count <= 0) return;
    font_ = x11_.load_font(display_, names[0]);
    x11_.free_font_names(names);
    if (font_ != 0) x11_.set_font(display_, gc_, font_);
  }

  void refresh_project_files() {
    files_.clear();
    std::error_code ec;
    const auto options = std::filesystem::directory_options::skip_permission_denied;
    std::filesystem::recursive_directory_iterator it(root_path_, options, ec);
    const std::filesystem::recursive_directory_iterator end;
    while (!ec && it != end && files_.size() < MAX_PROJECT_FILES) {
      const std::filesystem::directory_entry entry = *it;
      if (entry.is_directory(ec)) {
        if (should_skip_directory(entry.path())) it.disable_recursion_pending();
      } else if (entry.is_regular_file(ec)) {
        const std::uintmax_t size = entry.file_size(ec);
        if (!ec && size <= MAX_FILE_BYTES) {
          files_.push_back({entry.path(), display_label(entry.path()), "", {}, false});
        }
      }
      it.increment(ec);
      if (ec) ec.clear();
    }
    std::sort(files_.begin(), files_.end(), [](const EditorFile& left, const EditorFile& right) {
      return left.label < right.label;
    });
    if (active_file_ >= files_.size()) active_file_ = {};
  }

  bool should_skip_directory(const std::filesystem::path& path) const {
    const std::string name = path.filename().string();
    return name == ".git" || name == "build" || name == "vendor" || name == ".venv" ||
           name == ".state" || name == "dist" || name == "__pycache__";
  }

  std::string display_label(const std::filesystem::path& path) const {
    std::error_code ec;
    const std::filesystem::path rel = std::filesystem::relative(path, root_path_, ec);
    return ec ? path.filename().string() : rel.generic_string();
  }

  std::string root_label() const { return root_path_.generic_string(); }

  void load_palette() {
    palette_.background = color("#111315", black_);
    palette_.topbar = color("#15191d", black_);
    palette_.panel = color("#191d21", black_);
    palette_.editor = color("#0f1215", black_);
    palette_.border = color("#343b42", white_);
    palette_.text = color("#e6edf3", white_);
    palette_.muted = color("#9aa7b2", white_);
    palette_.accent = color("#4f8cff", white_);
    palette_.ok = color("#37a66b", white_);
    palette_.warn = color("#d8892c", white_);
  }

  unsigned long color(const char* spec, unsigned long fallback) {
    XColor parsed{};
    if (x11_.parse_color(display_, colormap_, spec, &parsed) == 0) return fallback;
    if (x11_.alloc_color(display_, colormap_, &parsed) == 0) return fallback;
    return parsed.pixel;
  }

  bool handle_event(const XEvent& event) {
    if (prompt_window_ != 0 && event.xany.window == prompt_window_) {
      return handle_prompt_event(event);
    }
    if (picker_window_ != 0 && event.xany.window == picker_window_) {
      return handle_picker_event(event);
    }

    switch (event.type) {
      case kExpose:
        draw();
        return true;
      case kConfigureNotify:
        width_ = event.xconfigure.width;
        height_ = event.xconfigure.height;
        draw();
        return true;
      case kKeyPress:
        return handle_key(event.xkey);
      case kButtonPress:
        return handle_click(event.xbutton);
      case kClientMessage:
        if (static_cast<Atom>(event.xclient.data.l[0]) == wm_delete_) return false;
        return true;
      default:
        return true;
    }
  }

  bool handle_picker_event(const XEvent& event) {
    switch (event.type) {
      case kExpose:
        draw_file_picker();
        return true;
      case kConfigureNotify:
        picker_width_ = event.xconfigure.width;
        picker_height_ = event.xconfigure.height;
        draw_file_picker();
        return true;
      case kButtonPress:
        return handle_picker_click(event.xbutton);
      case kKeyPress:
        return handle_picker_key(event.xkey);
      case kClientMessage:
        if (static_cast<Atom>(event.xclient.data.l[0]) == wm_delete_) {
          close_file_picker();
        }
        return true;
      default:
        return true;
    }
  }

  bool handle_prompt_event(const XEvent& event) {
    switch (event.type) {
      case kExpose:
        x11_.set_input_focus(display_, prompt_window_, X_REVERT_TO_PARENT, X_CURRENT_TIME);
        draw_prompt_window();
        return true;
      case kConfigureNotify:
        prompt_width_ = event.xconfigure.width;
        prompt_height_ = event.xconfigure.height;
        draw_prompt_window();
        return true;
      case kButtonPress:
        return handle_prompt_click(event.xbutton);
      case kKeyPress:
        return handle_prompt_key(event.xkey);
      case kClientMessage:
        if (static_cast<Atom>(event.xclient.data.l[0]) == wm_delete_) {
          close_prompt_window();
        }
        return true;
      default:
        return true;
    }
  }

  bool handle_key(XKeyEvent key_event) {
    char buffer[KEY_LOOKUP_BUFFER_SIZE]{};
    KeySym key{};
    const int length = x11_.lookup_string(&key_event, buffer, sizeof(buffer), &key, nullptr);

    if (key == kEscape) return false;
    if (matches_binding(key_event, key, buffer, length, config_.keys.prompt)) {
      open_prompt_window();
      draw();
      return true;
    }
    if (matches_binding(key_event, key, buffer, length, config_.keys.focus_editor)) {
      set_focus(FocusPane::Editor);
      draw();
      return true;
    }
    if (matches_binding(key_event, key, buffer, length, config_.keys.focus_notice)) {
      set_focus(FocusPane::Notice);
      draw();
      return true;
    }
    if (matches_binding(key_event, key, buffer, length, config_.keys.open_files)) {
      open_file_picker();
      draw();
      return true;
    }
    if (matches_binding(key_event, key, buffer, length, config_.keys.save)) {
      save_active();
      draw();
      return true;
    }
    if (matches_binding(key_event, key, buffer, length, config_.keys.demo)) {
      run_core_demo();
      draw();
      return true;
    }
    if (matches_binding(key_event, key, buffer, length, config_.keys.quit)) {
      status_ = "closing";
      draw();
      return false;
    }

    if (focus_pane_ != FocusPane::Editor) {
      draw();
      return true;
    }

    EditorFile* file = current_file();
    if (file == nullptr) return true;
    if (key == kBackSpace) {
      if (file->cursor > 0 && !file->content.empty()) {
        file->content.erase(file->cursor - 1, 1);
        --file->cursor;
        file->dirty = true;
        status_ = "editing " + file->label;
      }
    } else if (key == kReturn || key == kKeypadEnter) {
      insert_text("\n");
    } else if (length > 0) {
      std::string text(buffer, static_cast<std::size_t>(length));
      bool printable = true;
      for (unsigned char ch : text) {
        if (ch < FIRST_PRINTABLE_ASCII && ch != '\t') printable = false;
      }
      if (printable) insert_text(text);
    }

    draw();
    return true;
  }

  bool handle_picker_key(XKeyEvent key_event) {
    char buffer[KEY_LOOKUP_BUFFER_SIZE]{};
    KeySym key{};
    const int length = x11_.lookup_string(&key_event, buffer, sizeof(buffer), &key, nullptr);
    if (key == kEscape) {
      close_file_picker();
      return true;
    }
    if (key == X_KEY_UP && picker_selected_ > 0) {
      --picker_selected_;
      draw_file_picker();
      return true;
    }
    if (key == X_KEY_DOWN && picker_selected_ + 1 < files_.size()) {
      ++picker_selected_;
      draw_file_picker();
      return true;
    }
    if (key == kReturn || key == kKeypadEnter) {
      open_selected_picker_file();
      return true;
    }
    if ((key_event.state & kControlMask) != 0U && length == 1 &&
        (buffer[0] == 'q' || buffer[0] == 'Q')) {
      close_file_picker();
      return true;
    }
    return true;
  }

  bool handle_prompt_key(XKeyEvent key_event) {
    char buffer[KEY_LOOKUP_BUFFER_SIZE]{};
    KeySym key{};
    const int length = x11_.lookup_string(&key_event, buffer, sizeof(buffer), &key, nullptr);
    if (key == kEscape) {
      close_prompt_window();
      return true;
    }
    if (key == kBackSpace) {
      if (!prompt_input_.empty()) prompt_input_.pop_back();
      draw_prompt_window();
      return true;
    }
    if (key == kReturn || key == kKeypadEnter) {
      run_prompt_command();
      draw_prompt_window();
      return true;
    }
    if ((key_event.state & kControlMask) != 0U && length == 1) {
      if (buffer[0] == 'q' || buffer[0] == 'Q') {
        close_prompt_window();
        return true;
      }
      if (buffer[0] == 'l' || buffer[0] == 'L') {
        prompt_lines_.clear();
        draw_prompt_window();
        return true;
      }
    }
    if (length > 0) {
      std::string text(buffer, static_cast<std::size_t>(length));
      bool printable = true;
      for (unsigned char ch : text) {
        if (ch < FIRST_PRINTABLE_ASCII && ch != '\t') printable = false;
      }
      if (printable) {
        prompt_input_ += text;
        draw_prompt_window();
      }
    }
    return true;
  }

  bool handle_click(const XButtonEvent& event) {
    if (event.button != kButton1) return true;
    for (const HitRegion& hit : hits_) {
      if (!contains(hit.rect, event.x, event.y)) continue;
      switch (hit.action) {
        case HitAction::OpenFilesWindow:
          open_file_picker();
          break;
        case HitAction::OpenPromptWindow:
          open_prompt_window();
          break;
        case HitAction::SplitView:
          status_ = "split view is deferred";
          break;
        case HitAction::SaveFile:
          save_active();
          focus_pane_ = FocusPane::Editor;
          break;
        case HitAction::RunCore:
          run_core_demo();
          focus_pane_ = FocusPane::Notice;
          break;
        case HitAction::CloseWindow:
          status_ = "closing";
          draw();
          return false;
        case HitAction::ClosePicker:
        case HitAction::ClosePrompt:
        case HitAction::RunPrompt:
        case HitAction::SelectFile:
          break;
      }
      draw();
      return true;
    }
    return true;
  }

  bool handle_picker_click(const XButtonEvent& event) {
    if (event.button != kButton1) return true;
    for (const HitRegion& hit : picker_hits_) {
      if (!contains(hit.rect, event.x, event.y)) continue;
      if (hit.action == HitAction::ClosePicker) {
        close_file_picker();
        return true;
      }
      if (hit.action == HitAction::SelectFile && hit.index >= 0) {
        open_file(static_cast<std::size_t>(hit.index), true);
        close_file_picker();
        draw();
        return true;
      }
    }
    return true;
  }

  bool handle_prompt_click(const XButtonEvent& event) {
    if (event.button == X_BUTTON_SCROLL_UP) {
      scroll_prompt_output(PROMPT_SCROLL_STEP_LINES);
      draw_prompt_window();
      return true;
    }
    if (event.button == X_BUTTON_SCROLL_DOWN) {
      scroll_prompt_output(-PROMPT_SCROLL_STEP_LINES);
      draw_prompt_window();
      return true;
    }
    if (event.button != kButton1) return true;
    for (const HitRegion& hit : prompt_hits_) {
      if (!contains(hit.rect, event.x, event.y)) continue;
      if (hit.action == HitAction::ClosePrompt) {
        close_prompt_window();
        return true;
      }
      if (hit.action == HitAction::RunPrompt) {
        run_prompt_command();
        draw_prompt_window();
        return true;
      }
    }
    return true;
  }

  bool matches_binding(const XKeyEvent& key_event, KeySym key, const char* buffer, int length,
                       const KeyBinding& binding) const {
    if (binding.ctrl && (key_event.state & kControlMask) == 0U) return false;
    if (binding.key == '@') {
      return key == X_KEY_AT ||
             (length == 1 && (buffer[0] == '@' || buffer[0] == PROMPT_SHORTCUT_NUL));
    }
    const char expected =
        static_cast<char>(std::tolower(static_cast<unsigned char>(binding.key)));
    if (length == 1 &&
        std::tolower(static_cast<unsigned char>(buffer[0])) == static_cast<int>(expected)) {
      return true;
    }
    if (key <= UINT8_MAX &&
        std::tolower(static_cast<unsigned char>(key)) == static_cast<int>(expected)) {
      return true;
    }
    return false;
  }

  void open_file_picker() {
    refresh_project_files();
    if (picker_selected_ >= files_.size()) picker_selected_ = {};
    if (picker_window_ != 0) {
      draw_file_picker();
      return;
    }

    const Window root = x11_.root_window(display_, screen_);
    picker_width_ = FILE_PICKER_WIDTH;
    picker_height_ = FILE_PICKER_HEIGHT;
    picker_window_ = x11_.create_simple_window(
        display_, root, DEFAULT_WINDOW_X + DEFAULT_WINDOW_WIDTH + FLOATING_WINDOW_GAP,
        DEFAULT_WINDOW_Y + ui::TOPBAR_HEIGHT, static_cast<unsigned int>(picker_width_),
        static_cast<unsigned int>(picker_height_), 1, palette_.border, palette_.background);
    const std::string title = config_.app_name + " Files";
    x11_.store_name(display_, picker_window_, title.c_str());
    x11_.select_input(display_, picker_window_,
                      kExposureMask | kStructureNotifyMask | kKeyPressMask | kButtonPressMask);
    x11_.set_wm_protocols(display_, picker_window_, &wm_delete_, 1);
    x11_.map_window(display_, picker_window_);
    status_ = "files window opened";
  }

  void close_file_picker() {
    if (display_ == nullptr || picker_window_ == 0) return;
    x11_.destroy_window(display_, picker_window_);
    picker_window_ = 0;
    picker_hits_.clear();
  }

  void open_selected_picker_file() {
    if (picker_selected_ < files_.size()) {
      open_file(picker_selected_, true);
      close_file_picker();
      draw();
    }
  }

  void open_file(std::size_t index, bool through_registry) {
    if (index >= files_.size()) return;
    active_file_ = index;
    EditorFile& file = files_[active_file_];
    if (!load_file(file)) return;
    file.cursor = file.content.size();
    focus_pane_ = FocusPane::Editor;
    if (through_registry) {
      last_notice_ = registry_.open_file(file.path.string());
      last_notice_kind_ = "file";
    }
    status_ = "opened " + file.label;
  }

  bool load_file(EditorFile& file) {
    std::ifstream in(file.path, std::ios::binary);
    if (!in) {
      status_ = "open failed " + file.label;
      return false;
    }
    file.content.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    file.dirty = false;
    return true;
  }

  void save_active() {
    EditorFile* file = current_file();
    if (file == nullptr) return;
    std::ofstream out(file->path, std::ios::binary | std::ios::trunc);
    if (!out) {
      status_ = "save failed " + file->label;
      return;
    }
    out << file->content;
    file->dirty = false;
    status_ = "saved " + file->label;
  }

  void open_prompt_window() {
    if (prompt_window_ != 0) {
      x11_.raise_window(display_, prompt_window_);
      x11_.set_input_focus(display_, prompt_window_, X_REVERT_TO_PARENT, X_CURRENT_TIME);
      draw_prompt_window();
      status_ = "prompt raised";
      return;
    }

    const Window root = x11_.root_window(display_, screen_);
    prompt_width_ = PROMPT_WINDOW_WIDTH;
    prompt_height_ = PROMPT_WINDOW_HEIGHT;
    prompt_window_ = x11_.create_simple_window(
        display_, root, DEFAULT_WINDOW_X + FLOATING_WINDOW_GAP,
        DEFAULT_WINDOW_Y + FLOATING_WINDOW_GAP,
        static_cast<unsigned int>(prompt_width_), static_cast<unsigned int>(prompt_height_), 1,
        palette_.border, palette_.background);
    const std::string title = config_.app_name + " Prompt";
    x11_.store_name(display_, prompt_window_, title.c_str());
    x11_.select_input(display_, prompt_window_,
                      kExposureMask | kStructureNotifyMask | kKeyPressMask | kButtonPressMask);
    x11_.set_wm_protocols(display_, prompt_window_, &wm_delete_, 1);
    prompt_lines_.clear();
    prompt_lines_.push_back("root: " + root_label());
    prompt_lines_.push_back("Enter runs command. Esc closes this prompt.");
    scroll_prompt_to_bottom();
    x11_.map_window(display_, prompt_window_);
    x11_.raise_window(display_, prompt_window_);
    focus_pane_ = FocusPane::Prompt;
    status_ = "prompt opened at " + root_label();
  }

  void close_prompt_window() {
    if (display_ == nullptr || prompt_window_ == 0) return;
    x11_.destroy_window(display_, prompt_window_);
    prompt_window_ = 0;
    prompt_hits_.clear();
    focus_pane_ = FocusPane::Editor;
  }

  void run_prompt_command() {
    if (prompt_input_.empty()) return;
    const std::string command = prompt_input_;
    append_prompt_line(shell_name() + " $ " + command);
    std::string output = run_prompt_shell_command(command);
    prompt_input_.clear();
    std::size_t start = 0;
    while (start <= output.size()) {
      const std::size_t end = output.find('\n', start);
      const std::string line =
          end == std::string::npos ? output.substr(start) : output.substr(start, end - start);
      if (!line.empty()) append_prompt_line(line);
      if (end == std::string::npos) break;
      start = end + 1;
    }
    status_ = "prompt command finished";
  }

  std::string run_prompt_shell_command(const std::string& command) {
    int pipe_fds[2]{};
    if (pipe(pipe_fds) != 0) return "pipe failed";
    const pid_t pid = fork();
    if (pid < 0) {
      close(pipe_fds[PIPE_READ_END]);
      close(pipe_fds[PIPE_WRITE_END]);
      return "fork failed";
    }
    if (pid == 0) {
      close(pipe_fds[PIPE_READ_END]);
      dup2(pipe_fds[PIPE_WRITE_END], STDOUT_FILENO);
      dup2(pipe_fds[PIPE_WRITE_END], STDERR_FILENO);
      close(pipe_fds[PIPE_WRITE_END]);
      std::error_code ec;
      std::filesystem::current_path(root_path_, ec);
      const std::string shell = shell_path();
      execlp(shell.c_str(), shell.c_str(), "-lc", command.c_str(), nullptr);
      _exit(COMMAND_EXEC_FAILURE_EXIT);
    }

    close(pipe_fds[PIPE_WRITE_END]);
    std::string output;
    std::array<char, COMMAND_READ_BUFFER_SIZE> buffer{};
    while (true) {
      const ssize_t count = read(pipe_fds[PIPE_READ_END], buffer.data(), buffer.size());
      if (count <= 0) break;
      output.append(buffer.data(), static_cast<std::size_t>(count));
    }
    close(pipe_fds[PIPE_READ_END]);
    int status = COMMAND_EXIT_STATUS_OK;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) != COMMAND_EXIT_STATUS_OK) {
      output += "exit " + std::to_string(WEXITSTATUS(status)) + "\n";
    } else if (WIFSIGNALED(status)) {
      output += "signal " + std::to_string(COMMAND_SIGNAL_STATUS_BASE + WTERMSIG(status)) + "\n";
    }
    return output.empty() ? "ok\n" : output;
  }

  void append_prompt_line(std::string line) {
    prompt_lines_.push_back(std::move(line));
    if (prompt_lines_.size() > MAX_PROMPT_LINES) prompt_lines_.erase(prompt_lines_.begin());
    scroll_prompt_to_bottom();
  }

  std::string shell_path() const {
    const char* shell = std::getenv("SHELL");
    if (shell == nullptr || shell[0] == '\0') return "/bin/bash";
    return shell;
  }

  std::string shell_name() const {
    const std::filesystem::path path{shell_path()};
    const std::string name = path.filename().string();
    return name.empty() ? shell_path() : name;
  }

  std::string shell_status_line() const {
    return "shell: " + shell_name() + " (" + shell_path() + ")";
  }

  int prompt_visible_rows() const {
    return std::max(1, (prompt_height_ - PROMPT_OUTPUT_Y - PROMPT_OUTPUT_BOTTOM_PAD) /
                           line_height());
  }

  std::size_t max_prompt_scroll_offset() const {
    const std::size_t visible = static_cast<std::size_t>(prompt_visible_rows());
    return prompt_lines_.size() > visible ? prompt_lines_.size() - visible : 0U;
  }

  void scroll_prompt_to_bottom() { prompt_scroll_offset_ = 0U; }

  void scroll_prompt_output(int delta) {
    const std::size_t max_offset = max_prompt_scroll_offset();
    if (delta > 0) {
      prompt_scroll_offset_ =
          std::min(max_offset, prompt_scroll_offset_ + static_cast<std::size_t>(delta));
      return;
    }
    const std::size_t amount = static_cast<std::size_t>(-delta);
    prompt_scroll_offset_ = amount > prompt_scroll_offset_ ? 0U : prompt_scroll_offset_ - amount;
  }

  EditorFile* current_file() {
    if (files_.empty() || active_file_ >= files_.size()) return nullptr;
    return &files_[active_file_];
  }

  const EditorFile* current_file() const {
    if (files_.empty() || active_file_ >= files_.size()) return nullptr;
    return &files_[active_file_];
  }

  void set_focus(FocusPane pane) {
    focus_pane_ = pane;
    status_ = "focus " + pane_name(pane);
  }

  std::string pane_name(FocusPane pane) const {
    switch (pane) {
      case FocusPane::Editor:
        return "Editor";
      case FocusPane::Notice:
        return "Notice";
      case FocusPane::Prompt:
        return "Prompt";
    }
    return "Editor";
  }

  std::string pane_title(FocusPane pane, std::string_view title) const {
    if (focus_pane_ != pane) return std::string(title);
    return "> " + std::string(title);
  }

  void run_core_demo() {
    editor_proto::WorkspaceRegistry registry;
    editor_proto::ScrollController scroll;
    registry.open_file("/Repo/src/main.cpp");
    last_notice_ = registry.open_file("\\repo\\src\\main.cpp");
    last_notice_kind_ = "file";
    const auto root_first = registry.add_directory_root(root_path_.string());
    const auto root_second = registry.add_directory_root(root_path_.string() + "/");
    const auto pan = scroll.middle_button_pan(DEMO_SCROLL_OFFSET, DEMO_POINTER_DELTA);
    status_ = notice_text("root", root_second) + " offset=" + std::to_string(static_cast<int>(pan.new_offset));
    (void)root_first;
  }

  void insert_text(std::string_view text) {
    EditorFile* file = current_file();
    if (file == nullptr) return;
    file->content.insert(file->cursor, text);
    file->cursor += text.size();
    file->dirty = true;
    status_ = "editing " + file->label;
  }

  void log(std::string message) { status_ = std::move(message); }

  void draw() {
    hits_.clear();
    fill(window_, {0, 0, width_, height_}, palette_.background);

    constexpr int topbar_height = ui::TOPBAR_HEIGHT;
    constexpr int status_height = ui::STATUS_HEIGHT;
    const int main_height = height_ - topbar_height - status_height;
    const int inspector_width =
        width_ < ui::INSPECTOR_COMPACT_THRESHOLD ? ui::INSPECTOR_COMPACT_WIDTH
                                                 : ui::INSPECTOR_DEFAULT_WIDTH;
    const int editor_width = width_ - inspector_width;

    draw_topbar({0, 0, width_, topbar_height});
    draw_editor({0, topbar_height, editor_width, main_height});
    draw_inspector({editor_width, topbar_height, inspector_width, main_height});
    draw_status({0, height_ - status_height, width_, status_height});
    x11_.flush(display_);
  }

  void draw_topbar(Rect rect) {
    fill(window_, rect, palette_.topbar);
    line(window_, rect.x, rect.y + rect.height - ui::BORDER_OFFSET, rect.x + rect.width,
         rect.y + rect.height - ui::BORDER_OFFSET, palette_.border);
    text(window_, rect.x + ui::TOOLBAR_TITLE_X, rect.y + ui::TOOLBAR_BASELINE,
         config_.app_name, palette_.text);

    int x = rect.x + ui::TOOLBAR_BUTTON_START_X;
    x = button(window_, hits_, {x, rect.y + ui::TOOLBAR_BUTTON_TOP, ui::FILES_BUTTON_WIDTH,
                               ui::TOOLBAR_BUTTON_HEIGHT},
               "Files", HitAction::OpenFilesWindow) +
        ui::SMALL_GAP;
    x = button(window_, hits_, {x, rect.y + ui::TOOLBAR_BUTTON_TOP, ui::STANDARD_BUTTON_WIDTH,
                               ui::TOOLBAR_BUTTON_HEIGHT},
               "Save", HitAction::SaveFile) +
        ui::SMALL_GAP;
    x = button(window_, hits_, {x, rect.y + ui::TOOLBAR_BUTTON_TOP, ui::PROMPT_BUTTON_WIDTH,
                               ui::TOOLBAR_BUTTON_HEIGHT},
               "Prompt", HitAction::OpenPromptWindow) +
        ui::SMALL_GAP;
    x = button(window_, hits_, {x, rect.y + ui::TOOLBAR_BUTTON_TOP, ui::DEMO_BUTTON_WIDTH,
                               ui::TOOLBAR_BUTTON_HEIGHT},
               "Demo", HitAction::RunCore) +
        ui::SMALL_GAP;
    button(window_, hits_, {x, rect.y + ui::TOOLBAR_BUTTON_TOP, ui::CLOSE_BUTTON_WIDTH,
                            ui::TOOLBAR_BUTTON_HEIGHT},
           "Close", HitAction::CloseWindow);
    text(window_, rect.x + rect.width - ui::WORKSPACE_LABEL_RIGHT_PAD, rect.y + ui::TOOLBAR_BASELINE,
         truncate(root_label(), ui::WORKSPACE_LABEL_RIGHT_PAD), palette_.muted);
  }

  void draw_editor(Rect rect) {
    fill(window_, rect, palette_.editor);
    if (focus_pane_ == FocusPane::Editor) {
      line(window_, rect.x, rect.y, rect.x + rect.width, rect.y, palette_.accent);
    }
    line(window_, rect.x + rect.width - ui::BORDER_OFFSET, rect.y,
         rect.x + rect.width - ui::BORDER_OFFSET, rect.y + rect.height, palette_.border);
    fill(window_, {rect.x, rect.y, rect.width, ui::TAB_HEADER_HEIGHT}, palette_.background);
    line(window_, rect.x, rect.y + ui::TAB_HEADER_BOTTOM, rect.x + rect.width,
         rect.y + ui::TAB_HEADER_BOTTOM, palette_.border);

    const EditorFile* file = current_file();
    const std::string title = file == nullptr ? "No file" : file->label + (file->dirty ? " *" : "");
    text(window_, rect.x + ui::TAB_LABEL_X, rect.y + ui::TAB_BASELINE,
         truncate(title, rect.width - ui::EDITOR_TEXT_WIDTH_PAD), palette_.text);

    if (file == nullptr) {
      text(window_, rect.x + ui::EDITOR_TEXT_X, rect.y + ui::EDITOR_FIRST_LINE_Y,
           config_.keys.open_files.label + " opens a floating file picker.", palette_.muted);
      return;
    }

    int y = rect.y + ui::EDITOR_FIRST_LINE_Y;
    int line_no = 1;
    std::size_t start = 0;
    const int max_lines = std::max(1, (rect.height - ui::EDITOR_BOTTOM_PAD) / line_height());
    while (start <= file->content.size() && line_no <= max_lines) {
      const std::size_t end = file->content.find('\n', start);
      const std::string_view line_text =
          end == std::string::npos
              ? std::string_view(file->content).substr(start)
              : std::string_view(file->content).substr(start, end - start);
      text(window_, rect.x + ui::EDITOR_LINE_NUMBER_X, y, std::to_string(line_no), palette_.muted);
      text(window_, rect.x + ui::EDITOR_TEXT_X, y,
           truncate(std::string(line_text), rect.width - ui::EDITOR_TEXT_WIDTH_PAD), palette_.text);
      if (end == std::string::npos) break;
      start = end + 1;
      y += line_height();
      ++line_no;
    }
  }

  void draw_inspector(Rect rect) {
    fill(window_, rect, palette_.panel);
    text(window_, rect.x + ui::TITLE_X, rect.y + ui::TITLE_BASELINE,
         pane_title(FocusPane::Notice, "File"), palette_.text);
    line(window_, rect.x, rect.y + ui::HEADER_BOTTOM, rect.x + rect.width,
         rect.y + ui::HEADER_BOTTOM, palette_.border);

    const EditorFile* file = current_file();
    Rect card{rect.x + ui::NOTICE_CARD_X, rect.y + ui::NOTICE_CARD_Y,
              rect.width - ui::NOTICE_CARD_WIDTH_PAD, ui::NOTICE_CARD_HEIGHT};
    fill(window_, card, palette_.background);
    line(window_, card.x, card.y, card.x + card.width, card.y,
         file != nullptr && file->dirty ? palette_.warn : palette_.ok);
    text(window_, card.x + ui::NOTICE_CARD_X, card.y + ui::NOTICE_TITLE_BASELINE,
         file == nullptr ? "No file selected" : (file->dirty ? "dirty" : "clean"), palette_.text);
    text(window_, card.x + ui::NOTICE_CARD_X, card.y + ui::NOTICE_PATH_BASELINE,
         file == nullptr ? root_label() : truncate(file->path.string(), card.width - ui::NOTICE_CARD_WIDTH_PAD),
         palette_.muted);
    text(window_, card.x + ui::NOTICE_CARD_X, card.y + ui::NOTICE_ACTIONS_BASELINE,
         truncate(status_, card.width - ui::NOTICE_CARD_WIDTH_PAD), palette_.muted);
    text(window_, rect.x + ui::TITLE_X, rect.y + ui::CORE_LABEL_BASELINE, "Root", palette_.text);
    text(window_, rect.x + ui::TITLE_X, rect.y + ui::CORE_DETAIL_BASELINE,
         truncate(root_label(), rect.width - ui::NOTICE_CARD_WIDTH_PAD), palette_.muted);
  }

  void draw_file_picker() {
    if (picker_window_ == 0) return;
    picker_hits_.clear();
    fill(picker_window_, {0, 0, picker_width_, picker_height_}, palette_.panel);
    text(picker_window_, ui::TITLE_X, FILE_PICKER_STATUS_BASELINE, "Files", palette_.text);
    button(picker_window_, picker_hits_,
           {FILE_PICKER_CLOSE_X, ui::TOOLBAR_BUTTON_TOP, FILE_PICKER_CLOSE_WIDTH,
            ui::TOOLBAR_BUTTON_HEIGHT},
           "Close", HitAction::ClosePicker);
    text(picker_window_, ui::TITLE_X, FILE_PICKER_ROOT_Y, truncate(root_label(), picker_width_),
         palette_.muted);
    line(picker_window_, 0, ui::HEADER_BOTTOM, picker_width_, ui::HEADER_BOTTOM, palette_.border);

    int y = FILE_PICKER_ROW_START_Y;
    const int max_rows = std::max(1, (picker_height_ - FILE_PICKER_ROW_START_Y) / ui::ROW_STEP);
    for (std::size_t i = 0; i < files_.size() && static_cast<int>(i) < max_rows; ++i) {
      Rect row{ui::ROW_X_INSET, y, picker_width_ - ui::ROW_WIDTH_INSET, ui::ROW_HEIGHT};
      if (i == picker_selected_) fill(picker_window_, row, palette_.accent);
      text(picker_window_, ui::FILE_ICON_X, y + ui::FILE_ROW_BASELINE, "F", palette_.muted);
      text(picker_window_, ui::FILE_LABEL_X, y + ui::FILE_ROW_BASELINE,
           truncate(files_[i].label, picker_width_ - ui::FILE_LABEL_WIDTH_PAD), palette_.text);
      picker_hits_.push_back({row, HitAction::SelectFile, static_cast<int>(i)});
      y += ui::ROW_STEP;
    }
    x11_.flush(display_);
  }

  void draw_prompt_window() {
    if (prompt_window_ == 0) return;
    prompt_hits_.clear();
    fill(prompt_window_, {0, 0, prompt_width_, prompt_height_}, palette_.panel);
    text(prompt_window_, ui::TITLE_X, FILE_PICKER_STATUS_BASELINE,
         pane_title(FocusPane::Prompt, "Prompt"), palette_.text);

    const int close_x =
        std::max(ui::TITLE_X, prompt_width_ - PROMPT_CLOSE_WIDTH - PROMPT_BUTTON_RIGHT_PAD);
    const int run_x = std::max(ui::TITLE_X, close_x - PROMPT_RUN_WIDTH - PROMPT_BUTTON_GAP);
    button(prompt_window_, prompt_hits_,
           {run_x, ui::TOOLBAR_BUTTON_TOP, PROMPT_RUN_WIDTH, ui::TOOLBAR_BUTTON_HEIGHT}, "Run",
           HitAction::RunPrompt);
    button(prompt_window_, prompt_hits_,
           {close_x, ui::TOOLBAR_BUTTON_TOP, PROMPT_CLOSE_WIDTH, ui::TOOLBAR_BUTTON_HEIGHT},
           "Close", HitAction::ClosePrompt);
    line(prompt_window_, 0, ui::HEADER_BOTTOM, prompt_width_, ui::HEADER_BOTTOM, palette_.border);

    const int content_width =
        std::max(PROMPT_MIN_CONTENT_WIDTH, prompt_width_ - ui::TITLE_X - ui::TITLE_X);
    text(prompt_window_, ui::TITLE_X, PROMPT_ROOT_Y, truncate("root: " + root_label(), content_width),
         palette_.muted);
    text(prompt_window_, ui::TITLE_X, PROMPT_SHELL_Y, truncate(shell_status_line(), content_width),
         palette_.muted);

    const Rect input{ui::TITLE_X, PROMPT_INPUT_Y, content_width, PROMPT_INPUT_HEIGHT};
    fill(prompt_window_, input, palette_.background);
    line(prompt_window_, input.x, input.y, input.x + input.width, input.y, palette_.border);
    line(prompt_window_, input.x, input.y + input.height - ui::BORDER_OFFSET,
         input.x + input.width, input.y + input.height - ui::BORDER_OFFSET, palette_.border);
    text(prompt_window_, input.x + ui::BUTTON_LABEL_X,
         input.y + ui::BUTTON_LABEL_BASELINE, truncate("> " + prompt_input_ + "_", content_width),
         palette_.text);

    const int max_rows = prompt_visible_rows();
    const std::size_t visible =
        std::min(prompt_lines_.size(), static_cast<std::size_t>(max_rows));
    prompt_scroll_offset_ = std::min(prompt_scroll_offset_, max_prompt_scroll_offset());
    const std::size_t end = prompt_lines_.size() - prompt_scroll_offset_;
    const std::size_t start = end > visible ? end - visible : 0U;
    int y = PROMPT_OUTPUT_Y;
    for (std::size_t i = start; i < end; ++i) {
      text(prompt_window_, ui::TITLE_X, y, truncate(prompt_lines_[i], content_width),
           palette_.muted);
      y += line_height();
    }
    x11_.flush(display_);
  }

  void draw_status(Rect rect) {
    fill(window_, rect, palette_.topbar);
    line(window_, rect.x, rect.y, rect.x + rect.width, rect.y, palette_.border);
    text(window_, rect.x + ui::STATUS_TEXT_X, rect.y + ui::STATUS_BASELINE,
         truncate(status_, rect.width - ui::STATUS_TEXT_WIDTH_PAD), palette_.muted);
    text(window_, rect.x + rect.width - ui::STATUS_HINT_RIGHT_PAD, rect.y + ui::STATUS_BASELINE,
         shortcut_hint(), palette_.muted);
  }

  int button(Window target, std::vector<HitRegion>& hits, Rect rect, std::string_view label,
             HitAction action) {
    fill(target, rect, palette_.panel);
    line(target, rect.x, rect.y, rect.x + rect.width, rect.y, palette_.border);
    line(target, rect.x, rect.y + rect.height - ui::BORDER_OFFSET, rect.x + rect.width,
         rect.y + rect.height - ui::BORDER_OFFSET, palette_.border);
    text(target, rect.x + ui::BUTTON_LABEL_X, rect.y + ui::BUTTON_LABEL_BASELINE,
         std::string(label), palette_.text);
    hits.push_back({rect, action, -1});
    return rect.x + rect.width;
  }

  void fill(Window target, Rect rect, unsigned long color_value) {
    if (rect.width <= 0 || rect.height <= 0) return;
    x11_.set_foreground(display_, gc_, color_value);
    x11_.fill_rectangle(display_, target, gc_, rect.x, rect.y, static_cast<unsigned int>(rect.width),
                        static_cast<unsigned int>(rect.height));
  }

  void line(Window target, int x1, int y1, int x2, int y2, unsigned long color_value) {
    x11_.set_foreground(display_, gc_, color_value);
    x11_.draw_line(display_, target, gc_, x1, y1, x2, y2);
  }

  void text(Window target, int x, int y, const std::string& value, unsigned long color_value) {
    if (value.empty()) return;
    x11_.set_foreground(display_, gc_, color_value);
    x11_.draw_string(display_, target, gc_, x, y, value.c_str(), static_cast<int>(value.size()));
  }

  std::string truncate(const std::string& value, int max_width) const {
    const int max_chars = std::max(1, max_width / char_width());
    if (static_cast<int>(value.size()) <= max_chars) return value;
    if (max_chars <= ui::ELLIPSIS_WIDTH) return value.substr(0, static_cast<std::size_t>(max_chars));
    return value.substr(0, static_cast<std::size_t>(max_chars - ui::ELLIPSIS_WIDTH)) + "...";
  }

  std::string shortcut_hint() const {
    return config_.keys.open_files.label + " files  " + config_.keys.save.label + " save  " +
           config_.keys.prompt.label + " prompt";
  }

  X11 x11_;
  Display* display_{};
  int screen_{};
  Window window_{};
  Window picker_window_{};
  Window prompt_window_{};
  GC gc_{};
  Colormap colormap_{};
  Atom wm_delete_{};
  Font font_{};
  unsigned long black_{};
  unsigned long white_{};
  Palette palette_{};

  AppConfig config_;
  std::filesystem::path root_path_;
  int width_{DEFAULT_WINDOW_WIDTH};
  int height_{DEFAULT_WINDOW_HEIGHT};
  int picker_width_{FILE_PICKER_WIDTH};
  int picker_height_{FILE_PICKER_HEIGHT};
  int prompt_width_{PROMPT_WINDOW_WIDTH};
  int prompt_height_{PROMPT_WINDOW_HEIGHT};
  std::size_t active_file_{};
  std::size_t picker_selected_{};
  std::size_t prompt_scroll_offset_{};
  FocusPane focus_pane_{FocusPane::Editor};
  std::vector<EditorFile> files_;
  std::vector<HitRegion> hits_;
  std::vector<HitRegion> picker_hits_;
  std::vector<HitRegion> prompt_hits_;
  std::vector<std::string> prompt_lines_;
  std::string prompt_input_;
  std::string status_{"ready"};
  std::string last_notice_kind_{"file"};
  editor_proto::DuplicateNotice last_notice_;
  editor_proto::WorkspaceRegistry registry_;
};

void print_usage() {
  std::cout << "usage: mado [--config path] [--root path|--test-workspace] "
               "[--update] [--no-restart] [--smoke]\n";
}

}  // namespace

int main(int argc, char** argv) {
  try {
    std::filesystem::path config_path = default_config_path();
    bool update = false;
    bool restart = true;
    bool smoke = false;
    bool test_workspace = false;
    bool root_set = false;
    std::filesystem::path root_path = std::filesystem::current_path();

    for (int i = 1; i < argc; ++i) {
      const std::string arg = argv[i];
      if (arg == "--help") {
        print_usage();
        return 0;
      }
      if (arg == "--smoke") {
        smoke = true;
      } else if (arg == "--config" && i + 1 < argc) {
        config_path = argv[++i];
      } else if (arg == "--root" && i + 1 < argc) {
        root_path = argv[++i];
        root_set = true;
      } else if (arg == "--test-workspace") {
        test_workspace = true;
      } else if (arg == "--update") {
        update = true;
      } else if (arg == "--no-restart") {
        restart = false;
      } else {
        print_usage();
        return 1;
      }
    }

    AppConfig config = load_config(config_path);
    if (smoke) {
      std::cout << config.app_name << " native smoke ok\n";
      std::cout << demo_text() << "\n";
      return 0;
    }
    if (update) return run_update(config, restart);
    if (test_workspace && !root_set) {
      root_path =
          resolve_from_source(config.source_root, std::filesystem::path(std::string(TEST_WORKSPACE_DIR)));
    }

    NativeEditorApp app(std::move(config), std::filesystem::absolute(root_path));
    app.run();
  } catch (const std::exception& exc) {
    std::cerr << exc.what() << "\n";
    return 1;
  }

  return 0;
}
