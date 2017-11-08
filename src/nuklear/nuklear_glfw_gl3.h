/*
 * Nuklear - 1.32.0 - public domain
 * no warrenty implied; use at your own risk.
 * authored from 2015-2016 by Micha Mettke
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#ifndef NK_GLFW_GL3_H_
#define NK_GLFW_GL3_H_

#include <GLFW/glfw3.h>

#ifndef NK_GLFW_TEXT_MAX
#define NK_GLFW_TEXT_MAX 256
#endif

enum nk_glfw_init_state{
    NK_GLFW3_DEFAULT=0,
    NK_GLFW3_INSTALL_CALLBACKS
};


typedef void(*ControlKeyCallback)(int key, int action);

NK_API struct nk_context*   nk_glfw3_init(GLFWwindow *win, enum nk_glfw_init_state, ControlKeyCallback);
NK_API void                 nk_glfw3_shutdown(void);
NK_API void                 nk_glfw3_font_stash_begin(struct nk_font_atlas **atlas);
NK_API void                 nk_glfw3_font_stash_end(void);
NK_API void                 nk_glfw3_new_frame(void);
NK_API void                 nk_glfw3_render(enum nk_anti_aliasing, int max_vertex_buffer, int max_element_buffer);

NK_API void                 nk_glfw3_device_destroy(void);
NK_API void                 nk_glfw3_device_create(void);

NK_API void                 nk_glfw3_char_callback(GLFWwindow *win, unsigned int codepoint);
NK_API void                 nk_gflw3_scroll_callback(GLFWwindow *win, double xoff, double yoff);
NK_API void                 nk_glfw3_mouse_button_callback(GLFWwindow *win, int button, int action, int mods);

NK_API bool					nk_glfw3_load_image(const char* path, struct nk_image* image);

enum key_flags {
	KEY_FLAG_DEFAULT = 0,
	KEY_FLAG_REPEATS = NK_FLAG(0),
	KEY_FLAG_CHECK_CTRL = NK_FLAG(1),
	KEY_FLAG_CTRL = NK_FLAG(2)	//This is only checked if the flag above is set
};

struct nk_glfw_keymapping {
	nk_keys nk_key;
	int glfw_key;
	nk_flags flags;
};

const nk_glfw_keymapping keymap[] = {
	//	NK_KEY						GLFW_KEY				
	{ NK_KEY_DEL,				GLFW_KEY_DELETE,		KEY_FLAG_DEFAULT | KEY_FLAG_REPEATS },
	{ NK_KEY_ENTER,				GLFW_KEY_ENTER,			KEY_FLAG_DEFAULT | KEY_FLAG_REPEATS },
	{ NK_KEY_TAB,				GLFW_KEY_TAB,			KEY_FLAG_DEFAULT | KEY_FLAG_REPEATS },
	{ NK_KEY_BACKSPACE,			GLFW_KEY_BACKSPACE,		KEY_FLAG_DEFAULT | KEY_FLAG_REPEATS },
	{ NK_KEY_UP,					GLFW_KEY_UP,			KEY_FLAG_DEFAULT | KEY_FLAG_REPEATS },
	{ NK_KEY_DOWN,				GLFW_KEY_DOWN,			KEY_FLAG_DEFAULT | KEY_FLAG_REPEATS },
	{ NK_KEY_TEXT_LINE_START,	GLFW_KEY_HOME,			KEY_FLAG_DEFAULT },
	{ NK_KEY_TEXT_LINE_END,		GLFW_KEY_END,			KEY_FLAG_DEFAULT },
	{ NK_KEY_SCROLL_DOWN,		GLFW_KEY_PAGE_DOWN,		KEY_FLAG_DEFAULT | KEY_FLAG_REPEATS },
	{ NK_KEY_SCROLL_UP,			GLFW_KEY_PAGE_UP,		KEY_FLAG_DEFAULT | KEY_FLAG_REPEATS },
	{ NK_KEY_COPY,				GLFW_KEY_C,				KEY_FLAG_DEFAULT | KEY_FLAG_CHECK_CTRL | KEY_FLAG_CTRL },
	{ NK_KEY_PASTE,				GLFW_KEY_V,				KEY_FLAG_DEFAULT | KEY_FLAG_CHECK_CTRL | KEY_FLAG_CTRL },
	{ NK_KEY_CUT,				GLFW_KEY_X,				KEY_FLAG_DEFAULT | KEY_FLAG_CHECK_CTRL | KEY_FLAG_CTRL },
	{ NK_KEY_TEXT_UNDO,			GLFW_KEY_Z,				KEY_FLAG_DEFAULT | KEY_FLAG_CHECK_CTRL | KEY_FLAG_CTRL },
	{ NK_KEY_TEXT_REDO,			GLFW_KEY_R,				KEY_FLAG_DEFAULT | KEY_FLAG_CHECK_CTRL | KEY_FLAG_CTRL },
	{ NK_KEY_TEXT_WORD_LEFT,		GLFW_KEY_LEFT,			KEY_FLAG_DEFAULT | KEY_FLAG_CHECK_CTRL | KEY_FLAG_CTRL },
	{ NK_KEY_TEXT_WORD_RIGHT,	GLFW_KEY_RIGHT,			KEY_FLAG_DEFAULT | KEY_FLAG_CHECK_CTRL | KEY_FLAG_CTRL },
	{ NK_KEY_LEFT,				GLFW_KEY_LEFT,			KEY_FLAG_DEFAULT | KEY_FLAG_CHECK_CTRL },
	{ NK_KEY_RIGHT,				GLFW_KEY_RIGHT,			KEY_FLAG_DEFAULT | KEY_FLAG_CHECK_CTRL }
};

struct nk_glfw_device {
	struct nk_buffer cmds;
	struct nk_draw_null_texture null;
	GLuint vbo, vao, ebo;
	GLuint prog;
	GLuint vert_shdr;
	GLuint frag_shdr;
	GLint attrib_pos;
	GLint attrib_uv;
	GLint attrib_col;
	GLint uniform_tex;
	GLint uniform_proj;
	GLuint font_tex;
};

struct nk_glfw_vertex {
	float position[2];
	float uv[2];
	nk_byte col[4];
};


struct nk_glfw {
	GLFWwindow *win;
	int width, height;
	int display_width, display_height;
	struct nk_glfw_device ogl;
	struct nk_context ctx;
	struct nk_font_atlas atlas;
	struct nk_vec2 fb_scale;
	unsigned int text[NK_GLFW_TEXT_MAX];
	int text_len;
	struct nk_vec2 scroll;
	double last_button_click;
	GLFWcursor* cursors[NK_CURSOR_COUNT];
	nk_style_cursor activeCursor;
	bool doubleClickDown;
	struct nk_vec2 doubleClickPos;
	ControlKeyCallback controlKeyCB;
	bool* activatedKeys;
	int numKeys;
};

#endif

