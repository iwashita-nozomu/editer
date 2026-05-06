// @dependency-start
// responsibility Provides the runnable native GUI prototype for the editor core.
// upstream implementation ../include/editor_proto/workspace_registry.hpp workspace and scroll API
// upstream implementation ../CMakeLists.txt builds and tests this executable
// downstream design README.md documents repository build entrypoints
// @dependency-end
#include "editor_proto/workspace_registry.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace {

struct Display;

using Atom = unsigned long;
using Bool = int;
using Colormap = unsigned long;
using Drawable = unsigned long;
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

constexpr long X_KEY_PRESS_MASK_SHIFT = 0;
constexpr long X_BUTTON_PRESS_MASK_SHIFT = 2;
constexpr long X_EXPOSURE_MASK_SHIFT = 15;
constexpr long X_STRUCTURE_NOTIFY_MASK_SHIFT = 17;

constexpr unsigned int X_CONTROL_MASK_SHIFT = 2;
constexpr unsigned int X_BUTTON_PRIMARY = 1U;

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
constexpr std::size_t MAX_TERMINAL_LOG_LINES = 64;
constexpr int CHECK_NATIVE_DURATION_MS = 500;

namespace ui {

constexpr int TOPBAR_HEIGHT = 44;
constexpr int STATUS_HEIGHT = 24;
constexpr int TERMINAL_COMPACT_THRESHOLD = 620;
constexpr int TERMINAL_COMPACT_HEIGHT = 118;
constexpr int TERMINAL_DEFAULT_HEIGHT = 150;
constexpr int SIDEBAR_COMPACT_THRESHOLD = 900;
constexpr int SIDEBAR_COMPACT_WIDTH = 220;
constexpr int SIDEBAR_DEFAULT_WIDTH = 250;
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
constexpr int OPEN_BUTTON_WIDTH = 70;
constexpr int STANDARD_BUTTON_WIDTH = 64;
constexpr int DEMO_BUTTON_WIDTH = 72;
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
constexpr int TERMINAL_TITLE_X = 12;
constexpr int TERMINAL_TITLE_BASELINE = 22;
constexpr int TERMINAL_CONTENT_TOP = 46;
constexpr int TERMINAL_CONTENT_TOP_PAD = 38;
constexpr int TERMINAL_TEXT_WIDTH_PAD = 24;
constexpr int STATUS_TEXT_X = 10;
constexpr int STATUS_BASELINE = 17;
constexpr int STATUS_TEXT_WIDTH_PAD = 190;
constexpr int STATUS_HINT_RIGHT_PAD = 150;
constexpr int BUTTON_LABEL_X = 10;
constexpr int BUTTON_LABEL_BASELINE = 19;
constexpr int APPROX_CHAR_WIDTH = 8;
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
    create_gc = load_symbol<GC (*)(Display*, Drawable, unsigned long, void*)>(library_, "XCreateGC");
    set_foreground = load_symbol<int (*)(Display*, GC, unsigned long)>(library_, "XSetForeground");
    fill_rectangle = load_symbol<int (*)(Display*, Drawable, GC, int, int, unsigned int,
                                          unsigned int)>(library_, "XFillRectangle");
    draw_string = load_symbol<int (*)(Display*, Drawable, GC, int, int, const char*, int)>(
        library_, "XDrawString");
    draw_line = load_symbol<int (*)(Display*, Drawable, GC, int, int, int, int)>(library_,
                                                                                 "XDrawLine");
    flush = load_symbol<int (*)(Display*)>(library_, "XFlush");
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
  GC (*create_gc)(Display*, Drawable, unsigned long, void*){};
  int (*set_foreground)(Display*, GC, unsigned long){};
  int (*fill_rectangle)(Display*, Drawable, GC, int, int, unsigned int, unsigned int){};
  int (*draw_string)(Display*, Drawable, GC, int, int, const char*, int){};
  int (*draw_line)(Display*, Drawable, GC, int, int, int, int){};
  int (*flush)(Display*){};
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
  OpenFile,
  AddRoot,
  SplitView,
  SaveFile,
  RunCore,
  SelectFile,
};

struct HitRegion {
  Rect rect;
  HitAction action;
  int index{-1};
};

struct EditorFile {
  std::string path;
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

class NativeEditorApp {
 public:
  NativeEditorApp() {
    files_.push_back({
        "/repo/src/main.cpp",
        "main.cpp",
        "#include <iostream>\n\nint main() {\n  std::cout << \"Editor Make\" << std::endl;\n  "
        "return 0;\n}\n",
        0,
        false,
    });
    files_.push_back({
        "/repo/src/editor_proto_cli.cpp",
        "editor_proto_cli.cpp",
        "// CLI prototype surface\n// Native GUI prototype shares the duplicate core.\n",
        0,
        false,
    });
    files_.push_back({
        "/repo/notes/prototype_execution_plan_ja.md",
        "prototype_execution_plan_ja.md",
        "# Prototype Plan\n\n- Native window shell\n- Directory panel\n- Editor pane\n- Shell pane\n",
        0,
        false,
    });
  }

  void run(int duration_ms) {
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

    x11_.store_name(display_, window_, "Editor Make Native Prototype");
    x11_.select_input(display_, window_,
                      kExposureMask | kStructureNotifyMask | kKeyPressMask | kButtonPressMask);
    wm_delete_ = x11_.intern_atom(display_, "WM_DELETE_WINDOW", 0);
    x11_.set_wm_protocols(display_, window_, &wm_delete_, 1);
    gc_ = x11_.create_gc(display_, window_, 0, nullptr);
    x11_.map_window(display_, window_);

    open_file(0, true);
    add_root();
    log("native X11 window ready");
    std::cout << "Editor GUI prototype: native X11 window opened\n";
    std::cout << "Press Ctrl+C in this terminal or close the window to stop.\n" << std::flush;

    const auto start = std::chrono::steady_clock::now();
    bool running = true;
    while (running) {
      while (x11_.pending(display_) > 0) {
        XEvent event{};
        x11_.next_event(display_, &event);
        running = handle_event(event);
        if (!running) break;
      }

      if (duration_ms >= 0) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
        if (elapsed.count() >= duration_ms) running = false;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(EVENT_LOOP_SLEEP_MS));
    }
  }

  ~NativeEditorApp() {
    if (display_ == nullptr) return;
    if (gc_ != nullptr) {
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
    unsigned long terminal{};
    unsigned long border{};
    unsigned long text{};
    unsigned long muted{};
    unsigned long accent{};
    unsigned long ok{};
    unsigned long warn{};
  };

  void load_palette() {
    palette_.background = color("#111315", black_);
    palette_.topbar = color("#15191d", black_);
    palette_.panel = color("#191d21", black_);
    palette_.editor = color("#0f1215", black_);
    palette_.terminal = color("#13171b", black_);
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
        handle_click(event.xbutton);
        return true;
      case kClientMessage:
        if (static_cast<Atom>(event.xclient.data.l[0]) == wm_delete_) return false;
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
    if ((key_event.state & kControlMask) != 0U && (length == 1) &&
        (buffer[0] == 's' || buffer[0] == 'S')) {
      save_active();
      draw();
      return true;
    }

    EditorFile& file = files_[active_file_];
    if (key == kBackSpace) {
      if (file.cursor > 0 && !file.content.empty()) {
        file.content.erase(file.cursor - 1, 1);
        --file.cursor;
        file.dirty = true;
        status_ = "editing " + file.path;
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

  void handle_click(const XButtonEvent& event) {
    if (event.button != kButton1) return;
    for (const HitRegion& hit : hits_) {
      if (!contains(hit.rect, event.x, event.y)) continue;
      switch (hit.action) {
        case HitAction::OpenFile:
          open_file(0, true);
          break;
        case HitAction::AddRoot:
          add_root();
          break;
        case HitAction::SplitView:
          ++split_count_;
          status_ = "split view " + std::to_string(split_count_);
          log(status_);
          break;
        case HitAction::SaveFile:
          save_active();
          break;
        case HitAction::RunCore:
          run_core_demo();
          break;
        case HitAction::SelectFile:
          if (hit.index >= 0 && static_cast<std::size_t>(hit.index) < files_.size()) {
            open_file(static_cast<std::size_t>(hit.index), true);
          }
          break;
      }
      draw();
      return;
    }
  }

  void open_file(std::size_t index, bool through_registry) {
    active_file_ = index;
    EditorFile& file = files_[active_file_];
    file.cursor = file.content.size();
    if (through_registry) {
      last_notice_ = registry_.open_file(file.path);
      last_notice_kind_ = "file";
      log(notice_text("file", last_notice_));
    }
    status_ = "focused " + file.path;
  }

  void add_root() {
    last_notice_ = registry_.add_directory_root("/Work/ProjectA/");
    last_notice_kind_ = "root";
    status_ = "workspace /work/projecta";
    log(notice_text("root", last_notice_));
  }

  void save_active() {
    EditorFile& file = files_[active_file_];
    file.dirty = false;
    status_ = "saved " + file.path;
    log(status_);
  }

  void run_core_demo() {
    editor_proto::WorkspaceRegistry registry;
    editor_proto::ScrollController scroll;
    registry.open_file("/Repo/src/main.cpp");
    last_notice_ = registry.open_file("\\repo\\src\\main.cpp");
    last_notice_kind_ = "file";
    const auto root_first = registry.add_directory_root("/Work/ProjectA/");
    const auto root_second = registry.add_directory_root("/work/projecta");
    const auto pan = scroll.middle_button_pan(DEMO_SCROLL_OFFSET, DEMO_POINTER_DELTA);
    status_ = "core demo refreshed";
    log(notice_text("file", last_notice_));
    log(notice_text("root:first", root_first));
    log(notice_text("root:second", root_second));
    log("scroll:pan offset=" + std::to_string(static_cast<int>(pan.new_offset)) +
        " velocity=" + std::to_string(static_cast<int>(pan.velocity)));
  }

  void insert_text(std::string_view text) {
    EditorFile& file = files_[active_file_];
    file.content.insert(file.cursor, text);
    file.cursor += text.size();
    file.dirty = true;
    status_ = "editing " + file.path;
  }

  void log(std::string message) {
    terminal_.push_back(std::move(message));
    if (terminal_.size() > MAX_TERMINAL_LOG_LINES) terminal_.erase(terminal_.begin());
  }

  void draw() {
    hits_.clear();
    fill({0, 0, width_, height_}, palette_.background);

    constexpr int topbar_height = ui::TOPBAR_HEIGHT;
    constexpr int status_height = ui::STATUS_HEIGHT;
    const int terminal_height =
        height_ < ui::TERMINAL_COMPACT_THRESHOLD ? ui::TERMINAL_COMPACT_HEIGHT
                                                 : ui::TERMINAL_DEFAULT_HEIGHT;
    const int main_height = height_ - topbar_height - terminal_height - status_height;
    const int sidebar_width =
        width_ < ui::SIDEBAR_COMPACT_THRESHOLD ? ui::SIDEBAR_COMPACT_WIDTH
                                               : ui::SIDEBAR_DEFAULT_WIDTH;
    const int inspector_width =
        width_ < ui::INSPECTOR_COMPACT_THRESHOLD ? ui::INSPECTOR_COMPACT_WIDTH
                                                 : ui::INSPECTOR_DEFAULT_WIDTH;
    const int editor_width = width_ - sidebar_width - inspector_width;

    draw_topbar({0, 0, width_, topbar_height});
    draw_sidebar({0, topbar_height, sidebar_width, main_height});
    draw_editor({sidebar_width, topbar_height, editor_width, main_height});
    draw_inspector({sidebar_width + editor_width, topbar_height, inspector_width, main_height});
    draw_terminal({0, topbar_height + main_height, width_, terminal_height});
    draw_status({0, height_ - status_height, width_, status_height});
    x11_.flush(display_);
  }

  void draw_topbar(Rect rect) {
    fill(rect, palette_.topbar);
    line(rect.x, rect.y + rect.height - ui::BORDER_OFFSET, rect.x + rect.width,
         rect.y + rect.height - ui::BORDER_OFFSET, palette_.border);
    text(rect.x + ui::TOOLBAR_TITLE_X, rect.y + ui::TOOLBAR_BASELINE, "Editor Make Native",
         palette_.text);

    int x = rect.x + ui::TOOLBAR_BUTTON_START_X;
    x = button({x, rect.y + ui::TOOLBAR_BUTTON_TOP, ui::OPEN_BUTTON_WIDTH,
                ui::TOOLBAR_BUTTON_HEIGHT},
               "Open", HitAction::OpenFile) +
        ui::SMALL_GAP;
    x = button({x, rect.y + ui::TOOLBAR_BUTTON_TOP, ui::STANDARD_BUTTON_WIDTH,
                ui::TOOLBAR_BUTTON_HEIGHT},
               "Root", HitAction::AddRoot) +
        ui::SMALL_GAP;
    x = button({x, rect.y + ui::TOOLBAR_BUTTON_TOP, ui::STANDARD_BUTTON_WIDTH,
                ui::TOOLBAR_BUTTON_HEIGHT},
               "Split", HitAction::SplitView) +
        ui::SMALL_GAP;
    x = button({x, rect.y + ui::TOOLBAR_BUTTON_TOP, ui::STANDARD_BUTTON_WIDTH,
                ui::TOOLBAR_BUTTON_HEIGHT},
               "Save", HitAction::SaveFile) +
        ui::SMALL_GAP;
    button({x, rect.y + ui::TOOLBAR_BUTTON_TOP, ui::DEMO_BUTTON_WIDTH,
            ui::TOOLBAR_BUTTON_HEIGHT},
           "Demo", HitAction::RunCore);
    text(rect.x + rect.width - ui::WORKSPACE_LABEL_RIGHT_PAD, rect.y + ui::TOOLBAR_BASELINE,
         "workspace /Work/ProjectA", palette_.muted);
  }

  void draw_sidebar(Rect rect) {
    fill(rect, palette_.panel);
    line(rect.x + rect.width - ui::BORDER_OFFSET, rect.y,
         rect.x + rect.width - ui::BORDER_OFFSET, rect.y + rect.height, palette_.border);
    text(rect.x + ui::TITLE_X, rect.y + ui::TITLE_BASELINE, "Project", palette_.text);
    line(rect.x, rect.y + ui::HEADER_BOTTOM, rect.x + rect.width, rect.y + ui::HEADER_BOTTOM,
         palette_.border);
    text(rect.x + ui::ROOT_LABEL_X, rect.y + ui::ROOT_LABEL_BASELINE, "/work/projecta",
         palette_.muted);

    int y = rect.y + ui::FILE_ROW_START_Y;
    for (std::size_t i = 0; i < files_.size(); ++i) {
      const bool active = i == active_file_;
      Rect row{rect.x + ui::ROW_X_INSET, y, rect.width - ui::ROW_WIDTH_INSET, ui::ROW_HEIGHT};
      if (active) fill(row, palette_.accent);
      text(rect.x + ui::FILE_ICON_X, y + ui::FILE_ROW_BASELINE, "F",
           active ? palette_.text : palette_.muted);
      text(rect.x + ui::FILE_LABEL_X, y + ui::FILE_ROW_BASELINE,
           truncate(files_[i].label, rect.width - ui::FILE_LABEL_WIDTH_PAD), palette_.text);
      hits_.push_back({row, HitAction::SelectFile, static_cast<int>(i)});
      y += ui::ROW_STEP;
    }
  }

  void draw_editor(Rect rect) {
    fill(rect, palette_.editor);
    line(rect.x + rect.width - ui::BORDER_OFFSET, rect.y,
         rect.x + rect.width - ui::BORDER_OFFSET, rect.y + rect.height, palette_.border);
    fill({rect.x, rect.y, rect.width, ui::TAB_HEADER_HEIGHT}, palette_.background);
    line(rect.x, rect.y + ui::TAB_HEADER_BOTTOM, rect.x + rect.width,
         rect.y + ui::TAB_HEADER_BOTTOM, palette_.border);

    int tab_x = rect.x + ui::TAB_START_X;
    for (std::size_t i = 0; i < files_.size(); ++i) {
      const bool active = i == active_file_;
      Rect tab{tab_x, rect.y + ui::TAB_TOP, ui::TAB_WIDTH, ui::TAB_HEIGHT};
      fill(tab, active ? palette_.accent : palette_.panel);
      text(tab.x + ui::TAB_LABEL_X, tab.y + ui::TAB_BASELINE,
           truncate(files_[i].label + (files_[i].dirty ? " *" : ""), ui::TAB_TEXT_WIDTH),
           palette_.text);
      hits_.push_back({tab, HitAction::SelectFile, static_cast<int>(i)});
      tab_x += ui::TAB_STEP;
    }

    const EditorFile& file = files_[active_file_];
    int y = rect.y + ui::EDITOR_FIRST_LINE_Y;
    int line_no = 1;
    std::size_t start = 0;
    const int max_lines = std::max(1, (rect.height - ui::EDITOR_BOTTOM_PAD) / ui::TEXT_LINE_HEIGHT);
    while (start <= file.content.size() && line_no <= max_lines) {
      const std::size_t end = file.content.find('\n', start);
      const std::string_view line_text =
          end == std::string::npos
              ? std::string_view(file.content).substr(start)
              : std::string_view(file.content).substr(start, end - start);
      text(rect.x + ui::EDITOR_LINE_NUMBER_X, y, std::to_string(line_no), palette_.muted);
      text(rect.x + ui::EDITOR_TEXT_X, y,
           truncate(std::string(line_text), rect.width - ui::EDITOR_TEXT_WIDTH_PAD), palette_.text);
      if (end == std::string::npos) break;
      start = end + 1;
      y += ui::TEXT_LINE_HEIGHT;
      ++line_no;
    }
  }

  void draw_inspector(Rect rect) {
    fill(rect, palette_.panel);
    text(rect.x + ui::TITLE_X, rect.y + ui::TITLE_BASELINE, "Duplicate Notice", palette_.text);
    line(rect.x, rect.y + ui::HEADER_BOTTOM, rect.x + rect.width, rect.y + ui::HEADER_BOTTOM,
         palette_.border);

    const bool duplicate = last_notice_.is_duplicate;
    Rect card{rect.x + ui::NOTICE_CARD_X, rect.y + ui::NOTICE_CARD_Y,
              rect.width - ui::NOTICE_CARD_WIDTH_PAD, ui::NOTICE_CARD_HEIGHT};
    fill(card, palette_.background);
    line(card.x, card.y, card.x + card.width, card.y, duplicate ? palette_.warn : palette_.ok);
    text(card.x + ui::NOTICE_CARD_X, card.y + ui::NOTICE_TITLE_BASELINE,
         last_notice_kind_ + (duplicate ? " duplicate" : " opened"),
         duplicate ? palette_.warn : palette_.ok);
    text(card.x + ui::NOTICE_CARD_X, card.y + ui::NOTICE_PATH_BASELINE,
         truncate(last_notice_.canonical_path, card.width - ui::NOTICE_CARD_WIDTH_PAD),
         palette_.text);
    text(card.x + ui::NOTICE_CARD_X, card.y + ui::NOTICE_ACTIONS_BASELINE,
         truncate(actions_text(last_notice_.actions), card.width - ui::NOTICE_CARD_WIDTH_PAD),
         palette_.muted);
    text(rect.x + ui::TITLE_X, rect.y + ui::CORE_LABEL_BASELINE, "Core", palette_.text);
    text(rect.x + ui::TITLE_X, rect.y + ui::CORE_DETAIL_BASELINE,
         "WorkspaceRegistry + ScrollController", palette_.muted);
  }

  void draw_terminal(Rect rect) {
    fill(rect, palette_.terminal);
    line(rect.x, rect.y, rect.x + rect.width, rect.y, palette_.border);
    text(rect.x + ui::TERMINAL_TITLE_X, rect.y + ui::TERMINAL_TITLE_BASELINE, "Shell",
         palette_.text);
    const int max_lines =
        std::max(1, (rect.height - ui::TERMINAL_CONTENT_TOP_PAD) / ui::TEXT_LINE_HEIGHT);
    const std::size_t start =
        terminal_.size() > static_cast<std::size_t>(max_lines)
            ? terminal_.size() - static_cast<std::size_t>(max_lines)
            : 0;
    int y = rect.y + ui::TERMINAL_CONTENT_TOP;
    for (std::size_t i = start; i < terminal_.size(); ++i) {
      text(rect.x + ui::TERMINAL_TITLE_X, y,
           truncate(terminal_[i], rect.width - ui::TERMINAL_TEXT_WIDTH_PAD), palette_.muted);
      y += ui::TEXT_LINE_HEIGHT;
    }
  }

  void draw_status(Rect rect) {
    fill(rect, palette_.topbar);
    line(rect.x, rect.y, rect.x + rect.width, rect.y, palette_.border);
    text(rect.x + ui::STATUS_TEXT_X, rect.y + ui::STATUS_BASELINE,
         truncate(status_, rect.width - ui::STATUS_TEXT_WIDTH_PAD), palette_.muted);
    text(rect.x + rect.width - ui::STATUS_HINT_RIGHT_PAD, rect.y + ui::STATUS_BASELINE,
         "Ctrl+S save  Esc quit", palette_.muted);
  }

  int button(Rect rect, std::string_view label, HitAction action) {
    fill(rect, palette_.panel);
    line(rect.x, rect.y, rect.x + rect.width, rect.y, palette_.border);
    line(rect.x, rect.y + rect.height - ui::BORDER_OFFSET, rect.x + rect.width,
         rect.y + rect.height - ui::BORDER_OFFSET, palette_.border);
    text(rect.x + ui::BUTTON_LABEL_X, rect.y + ui::BUTTON_LABEL_BASELINE, std::string(label),
         palette_.text);
    hits_.push_back({rect, action, -1});
    return rect.x + rect.width;
  }

  void fill(Rect rect, unsigned long color_value) {
    if (rect.width <= 0 || rect.height <= 0) return;
    x11_.set_foreground(display_, gc_, color_value);
    x11_.fill_rectangle(display_, window_, gc_, rect.x, rect.y, static_cast<unsigned int>(rect.width),
                        static_cast<unsigned int>(rect.height));
  }

  void line(int x1, int y1, int x2, int y2, unsigned long color_value) {
    x11_.set_foreground(display_, gc_, color_value);
    x11_.draw_line(display_, window_, gc_, x1, y1, x2, y2);
  }

  void text(int x, int y, const std::string& value, unsigned long color_value) {
    if (value.empty()) return;
    x11_.set_foreground(display_, gc_, color_value);
    x11_.draw_string(display_, window_, gc_, x, y, value.c_str(), static_cast<int>(value.size()));
  }

  std::string truncate(const std::string& value, int max_width) const {
    const int max_chars = std::max(1, max_width / ui::APPROX_CHAR_WIDTH);
    if (static_cast<int>(value.size()) <= max_chars) return value;
    if (max_chars <= ui::ELLIPSIS_WIDTH) return value.substr(0, static_cast<std::size_t>(max_chars));
    return value.substr(0, static_cast<std::size_t>(max_chars - ui::ELLIPSIS_WIDTH)) + "...";
  }

  X11 x11_;
  Display* display_{};
  int screen_{};
  Window window_{};
  GC gc_{};
  Colormap colormap_{};
  Atom wm_delete_{};
  unsigned long black_{};
  unsigned long white_{};
  Palette palette_{};

  int width_{DEFAULT_WINDOW_WIDTH};
  int height_{DEFAULT_WINDOW_HEIGHT};
  int split_count_{1};
  std::size_t active_file_{};
  std::vector<EditorFile> files_;
  std::vector<HitRegion> hits_;
  std::vector<std::string> terminal_;
  std::string status_{"ready"};
  std::string last_notice_kind_{"file"};
  editor_proto::DuplicateNotice last_notice_;
  editor_proto::WorkspaceRegistry registry_;
};

int parse_duration_ms(const std::string& value) {
  try {
    const int duration = std::stoi(value);
    if (duration < 0) throw std::out_of_range("negative duration");
    return duration;
  } catch (const std::exception& exc) {
    throw std::runtime_error(std::string("invalid duration: ") + exc.what());
  }
}

void print_usage() {
  std::cout << "usage: editor_gui_proto [--smoke|--check-native|--duration-ms MS]\n";
}

}  // namespace

int main(int argc, char** argv) {
  int duration_ms = -1;

  try {
    for (int i = 1; i < argc; ++i) {
      const std::string arg = argv[i];
      if (arg == "--help") {
        print_usage();
        return 0;
      }
      if (arg == "--smoke") {
        std::cout << "editor_gui_proto native smoke ok\n";
        std::cout << demo_text() << "\n";
        return 0;
      }
      if (arg == "--check-native") {
        duration_ms = CHECK_NATIVE_DURATION_MS;
        continue;
      }
      if (arg == "--duration-ms" && i + 1 < argc) {
        duration_ms = parse_duration_ms(argv[++i]);
        continue;
      }
      print_usage();
      return 1;
    }

    NativeEditorApp app;
    app.run(duration_ms);
  } catch (const std::exception& exc) {
    std::cerr << exc.what() << "\n";
    return 1;
  }

  return 0;
}
