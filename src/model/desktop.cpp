#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Box.H>

#define D_WINDOW_WIDTH 900
#define D_WINDOW_HEIGHT 600

namespace desktop
{
    void desktop_main()
    {
        Fl_Window win(D_WINDOW_WIDTH, D_WINDOW_HEIGHT, "GHCD");
        win.begin();

        /*
        Fl_Menu_Bar menu(0, 0, D_WINDOW_WIDTH, 25);
        menu.add("文件/上传", 0, nullptr, nullptr);
        menu.add("文件/下载", 0, nullptr, nullptr, FL_MENU_DIVIDER);
        menu.add("文件/退出", FL_CTRL + 'q', [](Fl_Widget *, void *)
                 { Fl::hide_all_windows(); });
        */

        Fl_Box placeholder(0, 25, D_WINDOW_WIDTH, D_WINDOW_HEIGHT - 25, "GHCD Desktop\n测试界面");
        placeholder.labelsize(20);
        placeholder.align(FL_ALIGN_CENTER);

        win.end();
        win.show();
        Fl::run();
    }
}