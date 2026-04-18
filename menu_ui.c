#include <stdio.h>
#include "menu_ui.h"

void ui_clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

static void print_banner(void) {
    printf(" /$$$$$$$$ /$$$$$$ /$$$$$$$$ /$$   /$$        /$$$$$$  /$$$$$$$$ /$$$$$$$  /$$    /$$ /$$$$$$$$ /$$$$$$$ \n");
    printf("| $$_____/|_  $$_/| $$_____/| $$  /$$/       /$$__  $$| $$_____/| $$__  $$| $$   | $$| $$_____/| $$__  $$\n");
    printf("| $$        | $$  | $$      | $$ /$$/       | $$  \\__/| $$      | $$  \\ $$| $$   | $$| $$      | $$  \\ $$\n");
    printf("| $$$$$     | $$  | $$$$$   | $$$$$/        |  $$$$$$ | $$$$$   | $$$$$$$/|  $$ / $$/| $$$$$   | $$$$$$$/\n");
    printf("| $$__/     | $$  | $$__/   | $$  $$         \\____  $$| $$__/   | $$__  $$ \\  $$ $$/ | $$__/   | $$__  $$\n");
    printf("| $$        | $$  | $$      | $$\\  $$        /$$  \\ $$| $$      | $$  \\ $$  \\  $$$/  | $$      | $$  \\ $$\n");
    printf("| $$       /$$$$$$| $$$$$$$$| $$ \\  $$      |  $$$$$$/| $$$$$$$$| $$  | $$   \\  $/   | $$$$$$$$| $$  | $$\n");
    printf("|__/      |______/|________/|__/  \\__/       \\______/ |________/|__/  |__/    \\_/    |________/|__/  |__/\n");
}

static void print_commands(int is_admin) {
    printf("Komandat e disponueshme:\n");
    printf("  /list                Listoji file-t ne server\n");
    printf("  /read   <filename>   Lexo permbajtjen e nje file-i\n");
    printf("  /search <fjala>      Kerko file sipas emrit\n");
    printf("  /info   <filename>   Shfaq madhesine dhe datat e file-it\n");
    if (is_admin) {
        printf("  /upload   <filename> Dergo nje file ne server\n");
        printf("  /download <filename> Shkarko nje file nga serveri\n");
        printf("  /delete   <filename> Fshi nje file ne server\n");
    }
    printf("  /help                Kujtese per komandat\n");
    printf("  /exit                Shkepute nga serveri\n");
}

void ui_render_dashboard(const char *server_ip, int port, int is_admin,
                         const char *last_result) {
    ui_clear_screen();
    print_banner();
    printf("\n");
    printf("klienti udp -> %s:%d\n", server_ip, port);
    printf("roli: %s\n\n", is_admin ? "admin" : "user");

    printf("----------------------------------------\n");
    print_commands(is_admin);
    printf("----------------------------------------\n");

    printf("\nRezultati i fundit:\n");
    if (last_result && last_result[0] != '\0') {
        printf("%s\n", last_result);
    } else {
        printf("(Asnje komande ende)\n");
    }

    printf("----------------------------------------\n");
    printf("\nkomanda > ");
    fflush(stdout);
}
