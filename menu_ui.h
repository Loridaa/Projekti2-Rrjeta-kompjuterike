#ifndef MENU_UI_H
#define MENU_UI_H

void ui_clear_screen(void);
void ui_render_dashboard(const char *server_ip, int port, int is_admin,
                         const char *last_result);

#endif
