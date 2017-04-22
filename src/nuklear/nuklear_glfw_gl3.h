/*
 * Nuklear - v1.17 - public domain
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
#include <cfloat>

enum nk_glfw_init_state{
    NK_GLFW3_DEFAULT=0,
    NK_GLFW3_INSTALL_CALLBACKS
};

typedef void(*GLFW_Key_Callback)(GLFWwindow* win, int key, int scancode, int action, int mods);

extern "C" {
	NK_API struct nk_context*   nk_glfw3_init(GLFWwindow *win, enum nk_glfw_init_state init_state, void(*unhandled_key_callback)(GLFWwindow* win, int key, int scancode, int action, int mods));
	NK_API void                 nk_glfw3_shutdown(void);
	NK_API void                 nk_glfw3_font_stash_begin(struct nk_font_atlas **atlas);
	NK_API void                 nk_glfw3_font_stash_end(void);
	NK_API void                 nk_glfw3_new_frame(void);
	NK_API void                 nk_glfw3_render(enum nk_anti_aliasing, int max_vertex_buffer, int max_element_buffer);

	NK_API void                 nk_glfw3_device_destroy(void);
	NK_API void                 nk_glfw3_device_create(void);

	NK_API void                 nk_glfw3_char_callback(GLFWwindow *win, unsigned int codepoint);
	NK_API void                 nk_gflw3_scroll_callback(GLFWwindow *win, double xoff, double yoff);
}

/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_GLFW_GL3_IMPLEMENTATION

#ifndef NK_GLFW_TEXT_MAX
#define NK_GLFW_TEXT_MAX 256
#endif

#ifndef NK_GLFW_DBL_CLICK_TIME
#define NK_GLFW_DBL_CLICK_TIME 0.2
#endif

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

struct nk_glfw_click {
	int x;
	int y;
	bool pressed;
};



static struct nk_glfw {
    GLFWwindow *win;
    int width, height;
    int display_width, display_height;
    struct nk_glfw_device ogl;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    struct nk_vec2 fb_scale;
    unsigned int text[NK_GLFW_TEXT_MAX];
    int text_len;
    float scroll;
	bool actionKeys[NK_KEY_MAX] = { false };
	nk_glfw_click mouse_buttons[NK_BUTTON_MAX];
	GLFW_Key_Callback key_cb;
	//State for double click support
	double lastLeftClickTime = -1;
	int lastLeftClickX;
	int lastLeftClickY;

} glfw;

#ifdef __APPLE__
  #define NK_SHADER_VERSION "#version 150\n"
#else
  #define NK_SHADER_VERSION "#version 300 es\n"
#endif

NK_API void
nk_glfw3_device_create(void)
{
    GLint status;
    static const GLchar *vertex_shader =
        NK_SHADER_VERSION
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 TexCoord;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main() {\n"
        "   Frag_UV = TexCoord;\n"
        "   Frag_Color = Color;\n"
        "   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
        "}\n";
    static const GLchar *fragment_shader =
        NK_SHADER_VERSION
        "precision mediump float;\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main(){\n"
        "   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    struct nk_glfw_device *dev = &glfw.ogl;
    nk_buffer_init_default(&dev->cmds);
    dev->prog = glCreateProgram();
    dev->vert_shdr = glCreateShader(GL_VERTEX_SHADER);
    dev->frag_shdr = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(dev->vert_shdr, 1, &vertex_shader, 0);
    glShaderSource(dev->frag_shdr, 1, &fragment_shader, 0);
    glCompileShader(dev->vert_shdr);
    glCompileShader(dev->frag_shdr);
    glGetShaderiv(dev->vert_shdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);
    glGetShaderiv(dev->frag_shdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);
    glAttachShader(dev->prog, dev->vert_shdr);
    glAttachShader(dev->prog, dev->frag_shdr);
    glLinkProgram(dev->prog);
    glGetProgramiv(dev->prog, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);

    dev->uniform_tex = glGetUniformLocation(dev->prog, "Texture");
    dev->uniform_proj = glGetUniformLocation(dev->prog, "ProjMtx");
    dev->attrib_pos = glGetAttribLocation(dev->prog, "Position");
    dev->attrib_uv = glGetAttribLocation(dev->prog, "TexCoord");
    dev->attrib_col = glGetAttribLocation(dev->prog, "Color");

    {
        /* buffer setup */
        GLsizei vs = sizeof(struct nk_glfw_vertex);
        size_t vp = offsetof(struct nk_glfw_vertex, position);
        size_t vt = offsetof(struct nk_glfw_vertex, uv);
        size_t vc = offsetof(struct nk_glfw_vertex, col);

        glGenBuffers(1, &dev->vbo);
        glGenBuffers(1, &dev->ebo);
        glGenVertexArrays(1, &dev->vao);

        glBindVertexArray(dev->vao);
        glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

        glEnableVertexAttribArray((GLuint)dev->attrib_pos);
        glEnableVertexAttribArray((GLuint)dev->attrib_uv);
        glEnableVertexAttribArray((GLuint)dev->attrib_col);

        glVertexAttribPointer((GLuint)dev->attrib_pos, 2, GL_FLOAT, GL_FALSE, vs, (void*)vp);
        glVertexAttribPointer((GLuint)dev->attrib_uv, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
        glVertexAttribPointer((GLuint)dev->attrib_col, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

NK_INTERN void
nk_glfw3_device_upload_atlas(const void *image, int width, int height)
{
    struct nk_glfw_device *dev = &glfw.ogl;
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, image);
}

NK_API void
nk_glfw3_device_destroy(void)
{
    struct nk_glfw_device *dev = &glfw.ogl;
    glDetachShader(dev->prog, dev->vert_shdr);
    glDetachShader(dev->prog, dev->frag_shdr);
    glDeleteShader(dev->vert_shdr);
    glDeleteShader(dev->frag_shdr);
    glDeleteProgram(dev->prog);
    glDeleteTextures(1, &dev->font_tex);
    glDeleteBuffers(1, &dev->vbo);
    glDeleteBuffers(1, &dev->ebo);
    nk_buffer_free(&dev->cmds);
}

NK_API void
nk_glfw3_render(enum nk_anti_aliasing AA, int max_vertex_buffer, int max_element_buffer)
{
    struct nk_glfw_device *dev = &glfw.ogl;
    GLfloat ortho[4][4] = {
        {2.0f, 0.0f, 0.0f, 0.0f},
        {0.0f,-2.0f, 0.0f, 0.0f},
        {0.0f, 0.0f,-1.0f, 0.0f},
        {-1.0f,1.0f, 0.0f, 1.0f},
    };
    ortho[0][0] /= (GLfloat)glfw.width;
    ortho[1][1] /= (GLfloat)glfw.height;

    /* setup global state */
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    /* setup program */
    glUseProgram(dev->prog);
    glUniform1i(dev->uniform_tex, 0);
    glUniformMatrix4fv(dev->uniform_proj, 1, GL_FALSE, &ortho[0][0]);
    glViewport(0,0,(GLsizei)glfw.display_width,(GLsizei)glfw.display_height);
    {
        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        void *vertices, *elements;
        const nk_draw_index *offset = NULL;

        /* allocate vertex and element buffer */
        glBindVertexArray(dev->vao);
        glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

        glBufferData(GL_ARRAY_BUFFER, max_vertex_buffer, NULL, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_element_buffer, NULL, GL_STREAM_DRAW);

        /* load draw vertices & elements directly into vertex + element buffer */
        vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        {
            /* fill convert configuration */
            struct nk_convert_config config;
            static const struct nk_draw_vertex_layout_element vertex_layout[] = {
                {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, position)},
                {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, uv)},
                {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_glfw_vertex, col)},
                {NK_VERTEX_LAYOUT_END}
            };
            NK_MEMSET(&config, 0, sizeof(config));
            config.vertex_layout = vertex_layout;
            config.vertex_size = sizeof(struct nk_glfw_vertex);
            config.vertex_alignment = NK_ALIGNOF(struct nk_glfw_vertex);
            config.null = dev->null;
            config.circle_segment_count = 22;
            config.curve_segment_count = 22;
            config.arc_segment_count = 22;
            config.global_alpha = 1.0f;
            config.shape_AA = AA;
            config.line_AA = AA;

            /* setup buffers to load vertices and elements */
            {struct nk_buffer vbuf, ebuf;
            nk_buffer_init_fixed(&vbuf, vertices, (size_t)max_vertex_buffer);
            nk_buffer_init_fixed(&ebuf, elements, (size_t)max_element_buffer);
            nk_convert(&glfw.ctx, &dev->cmds, &vbuf, &ebuf, &config);}
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        /* iterate over and execute each draw command */
        nk_draw_foreach(cmd, &glfw.ctx, &dev->cmds)
        {
            if (!cmd->elem_count) continue;
            glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            glScissor(
                (GLint)(cmd->clip_rect.x * glfw.fb_scale.x),
                (GLint)((glfw.height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * glfw.fb_scale.y),
                (GLint)(cmd->clip_rect.w * glfw.fb_scale.x),
                (GLint)(cmd->clip_rect.h * glfw.fb_scale.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        nk_clear(&glfw.ctx);
    }

    /* default OpenGL state */
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
}

NK_API void
nk_glfw3_key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
	if (action == GLFW_RELEASE) {
		return;
	}

	if (key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(win, true);
	}

	if (key > GLFW_KEY_ESCAPE && key != ' ') {
			switch (key) {
			case GLFW_KEY_DELETE:
				glfw.actionKeys[NK_KEY_DEL] = true;
				break;
			case GLFW_KEY_ENTER:
				glfw.actionKeys[NK_KEY_ENTER] = true;
				break;
			case GLFW_KEY_BACKSPACE:
				glfw.actionKeys[NK_KEY_BACKSPACE] = true;
				break;
			case GLFW_KEY_TAB:
				glfw.actionKeys[NK_KEY_TAB] = true;
				break;
			case GLFW_KEY_HOME:
				glfw.actionKeys[NK_KEY_TEXT_LINE_START] = true;
				break;
			case GLFW_KEY_END:
				glfw.actionKeys[NK_KEY_TEXT_LINE_END] = true;
				break;
			case GLFW_KEY_UP:
				glfw.actionKeys[NK_KEY_UP] = true;
				break;
			case GLFW_KEY_DOWN:
				glfw.actionKeys[NK_KEY_DOWN] = true;
				break;
			case GLFW_KEY_PAGE_UP:
				glfw.actionKeys[NK_KEY_SCROLL_UP] = true;
				break;
			case GLFW_KEY_PAGE_DOWN:
				glfw.actionKeys[NK_KEY_SCROLL_DOWN] = true;
				break;
			case GLFW_KEY_RIGHT_SHIFT:
			case GLFW_KEY_LEFT_SHIFT:
				glfw.actionKeys[NK_KEY_SHIFT] = true;
				break;
			default:
				glfw.key_cb(win, key, scancode, action, mods);
        return;
			}

	}
	//Keys modified by CTRL
	if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_C:
			glfw.actionKeys[NK_KEY_COPY] = true;
			break;
		case GLFW_KEY_V:
			glfw.actionKeys[NK_KEY_PASTE] = true;
			break;
		case GLFW_KEY_X:
			glfw.actionKeys[NK_KEY_CUT] = true;
			break;
		case GLFW_KEY_Z:
			glfw.actionKeys[NK_KEY_TEXT_UNDO] = true;
			break;
		case GLFW_KEY_R:
			glfw.actionKeys[NK_KEY_TEXT_REDO] = true;
			break;
		case GLFW_KEY_LEFT:
			glfw.actionKeys[NK_KEY_TEXT_WORD_LEFT] = true;
			break;
		case GLFW_KEY_RIGHT:
			glfw.actionKeys[NK_KEY_TEXT_WORD_RIGHT] = true;
			break;
		case GLFW_KEY_SPACE:
			glfw.actionKeys[NK_KEY_AUTOCOMPLETE] = true;
			break;
		default:
			glfw.key_cb(win, key, scancode, action, mods);
			return;
		}
	}
	else {
		switch (key) {
		case GLFW_KEY_LEFT:
			glfw.actionKeys[NK_KEY_LEFT] = true;
			break;
		case GLFW_KEY_RIGHT:
			glfw.actionKeys[NK_KEY_RIGHT] = true;
			break;
    case GLFW_KEY_SPACE:
      {
        if(glfw.text_len < NK_GLFW_TEXT_MAX) {
          glfw.text[glfw.text_len++] = ' ';
        }
      }
		default:
			glfw.key_cb(win, key, scancode, action, mods);
			return;
		}
	}
}

NK_API void
nk_glfw3_mouse_button_callback(GLFWwindow* win, int button, int action, int mods) {
	int x, y;
	double mX, mY;
	glfwGetCursorPos(win, &mX, &mY);
	x = (int)mX;
	y = (int)mY;
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		glfw.mouse_buttons[NK_BUTTON_LEFT].pressed = (action == GLFW_PRESS);
		glfw.mouse_buttons[NK_BUTTON_LEFT].x = x;
		glfw.mouse_buttons[NK_BUTTON_LEFT].y = y;

		if (action == GLFW_PRESS) {
			if (glfwGetTime() - glfw.lastLeftClickTime < NK_GLFW_DBL_CLICK_TIME) {
				glfw.mouse_buttons[NK_BUTTON_DOUBLE_CLICK].pressed = true;
				glfw.mouse_buttons[NK_BUTTON_DOUBLE_CLICK].x = glfw.lastLeftClickX;
				glfw.mouse_buttons[NK_BUTTON_DOUBLE_CLICK].y = glfw.lastLeftClickY;
				glfw.lastLeftClickTime = DBL_MIN;
			}
			else {
				glfw.mouse_buttons[NK_BUTTON_DOUBLE_CLICK].pressed = false;
				glfw.lastLeftClickTime = glfwGetTime();
				glfw.lastLeftClickX = x;
				glfw.lastLeftClickY = y;
			}
		}
		else {
			glfw.mouse_buttons[NK_BUTTON_DOUBLE_CLICK].pressed = false;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		glfw.mouse_buttons[NK_BUTTON_RIGHT].pressed = (action == GLFW_PRESS);
		glfw.mouse_buttons[NK_BUTTON_LEFT].x = x;
		glfw.mouse_buttons[NK_BUTTON_LEFT].y = y;
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		glfw.mouse_buttons[NK_BUTTON_MIDDLE].pressed = (action == GLFW_PRESS);
		glfw.mouse_buttons[NK_BUTTON_LEFT].x = x;
		glfw.mouse_buttons[NK_BUTTON_LEFT].y = y;
	}
	else {

	}
}

NK_API void
nk_glfw3_char_callback(GLFWwindow *win, unsigned int codepoint)
{
	if (codepoint == ' ') {
		return;
	}
    (void)win;
    if (glfw.text_len < NK_GLFW_TEXT_MAX)
        glfw.text[glfw.text_len++] = codepoint;
}

NK_API void
nk_gflw3_scroll_callback(GLFWwindow *win, double xoff, double yoff)
{
    (void)win; (void)xoff;
    glfw.scroll += (float)yoff;
}

NK_INTERN void
nk_glfw3_clipbard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    const char *text = glfwGetClipboardString(glfw.win);
    if (text) nk_textedit_paste(edit, text, nk_strlen(text));
    (void)usr;
}

NK_INTERN void
nk_glfw3_my_clipbard_copy(nk_handle usr, TextBuffer* buffer, nk_my_vec2i start, nk_my_vec2i end)
{
    char *str = 0;
    (void)usr;
	int len = 0;
	int startByte = nk_my_bytes_to_glyph(&buffer->lines[start.x], start.y);
	int endByte = nk_my_bytes_to_glyph(&buffer->lines[end.x], end.y);
	if (start.x == end.x) {
		len = endByte - startByte;
		str = (char*)malloc((size_t)len + 1);
		if (!str) return;
		memcpy(str, &buffer->lines[start.x].text[startByte], (size_t)len);
		str[len] = '\0';
		glfwSetClipboardString(glfw.win, str);
		free(str);
	}
	else {
		len = buffer->lines[start.x].len - startByte;
		len++;//For the \n
		for (int i = start.x + 1; i < end.x; i++) {
			len += buffer->lines[i].len;
			len++; //For the \n
		}
		len += endByte;

		str = (char*)malloc((size_t)len + 1);
		if (!str) return;

		int offset = 0;
		int copyLen = buffer->lines[start.x].len - startByte;
		memcpy(str, &buffer->lines[start.x].text[startByte], copyLen);
		offset += copyLen;
		str[offset] = '\n';
		offset++;
		for (int i = start.x + 1; i < end.x; i++) {
			copyLen = buffer->lines[i].len;
			printf("Stuff: %s\n", buffer->lines[i].text);
			memcpy(&str[offset], buffer->lines[i].text, copyLen);
			offset += copyLen;
			str[offset] = '\n';
			offset++;
		}
		copyLen = endByte;
		memcpy(str + offset, buffer->lines[end.x].text, copyLen);
		offset += copyLen;
		str[offset] = '\0';
		glfwSetClipboardString(glfw.win, str);
		free(str);
	}

}

NK_INTERN void
nk_glfw3_my_clipbard_paste(nk_handle usr, struct nk_my_text_edit *edit)
{
	const char *text = glfwGetClipboardString(glfw.win);
	if (text) nk_my_textedit_paste(edit, text, nk_strlen(text));
	(void)usr;
}

NK_INTERN void
nk_glfw3_clipbard_copy(nk_handle usr, const char *text, int len)
{
	char *str = 0;
	(void)usr;
	if (!len) return;
	str = (char*)malloc((size_t)len + 1);
	if (!str) return;
	memcpy(str, text, (size_t)len);
	str[len] = '\0';
	glfwSetClipboardString(glfw.win, str);
	free(str);
}

NK_API struct nk_context*
nk_glfw3_init(GLFWwindow *win, enum nk_glfw_init_state init_state, GLFW_Key_Callback key_cb)
{
    glfw.win = win;
    if (init_state == NK_GLFW3_INSTALL_CALLBACKS) {
        glfwSetScrollCallback(win, nk_gflw3_scroll_callback);
        glfwSetCharCallback(win, nk_glfw3_char_callback);
		glfwSetKeyCallback(win, nk_glfw3_key_callback);
		glfwSetMouseButtonCallback(win, nk_glfw3_mouse_button_callback);
    }

    nk_init_default(&glfw.ctx, 0);
    glfw.ctx.clip.copy = nk_glfw3_clipbard_copy;
    glfw.ctx.clip.paste = nk_glfw3_clipbard_paste;
    glfw.ctx.clip.userdata = nk_handle_ptr(0);

	glfw.ctx.my_clip.copy = nk_glfw3_my_clipbard_copy;
	glfw.ctx.my_clip.paste = nk_glfw3_my_clipbard_paste;
	glfw.ctx.my_clip.userdata = nk_handle_ptr(0);

	glfw.key_cb = key_cb;
    nk_glfw3_device_create();
    return &glfw.ctx;
}

NK_API void
nk_glfw3_font_stash_begin(struct nk_font_atlas **atlas)
{
    nk_font_atlas_init_default(&glfw.atlas);
    nk_font_atlas_begin(&glfw.atlas);
    *atlas = &glfw.atlas;
}

NK_API void
nk_glfw3_font_stash_end(void)
{
    const void *image; int w, h;
    image = nk_font_atlas_bake(&glfw.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    nk_glfw3_device_upload_atlas(image, w, h);
    nk_font_atlas_end(&glfw.atlas, nk_handle_id((int)glfw.ogl.font_tex), &glfw.ogl.null);
    if (glfw.atlas.default_font)
        nk_style_set_font(&glfw.ctx, &glfw.atlas.default_font->handle);
}

NK_API void
nk_glfw3_new_frame(void)
{
	int i;
	double x, y;
	struct nk_context *ctx = &glfw.ctx;
	struct GLFWwindow *win = glfw.win;

	glfwGetWindowSize(win, &glfw.width, &glfw.height);
	glfwGetFramebufferSize(win, &glfw.display_width, &glfw.display_height);
	glfw.fb_scale.x = (float)glfw.display_width / (float)glfw.width;
	glfw.fb_scale.y = (float)glfw.display_height / (float)glfw.height;

	nk_input_begin(ctx);
	for (i = 0; i < glfw.text_len; ++i)
		nk_input_unicode(ctx, glfw.text[i]);

	glfw.actionKeys[NK_KEY_SHIFT] = (glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) ||
									(glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
	for (i = 0; i < NK_KEY_MAX; i++) {
		nk_input_key(ctx, (nk_keys)i, glfw.actionKeys[i]);
		glfw.actionKeys[i] = false;
	}

	/* optional grabbing behavior */
	if (ctx->input.mouse.grab)
		glfwSetInputMode(glfw.win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	else if (ctx->input.mouse.ungrab)
		glfwSetInputMode(glfw.win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);


    glfwGetCursorPos(win, &x, &y);
    nk_input_motion(ctx, (int)x, (int)y);
    if (ctx->input.mouse.grabbed) {
        glfwSetCursorPos(glfw.win, ctx->input.mouse.prev.x, ctx->input.mouse.prev.y);
        ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
        ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
    }

	for (int i = 0; i < NK_BUTTON_MAX; i++) {
		nk_input_button(ctx, (nk_buttons)i, glfw.mouse_buttons[i].x, glfw.mouse_buttons[i].y, glfw.mouse_buttons[i].pressed);
	}
	glfw.mouse_buttons[NK_BUTTON_DOUBLE_CLICK].pressed = false;

    //nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
   //nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    //nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    nk_input_scroll(ctx, glfw.scroll);
    nk_input_end(&glfw.ctx);
    glfw.text_len = 0;
    glfw.scroll = 0;
}

NK_API
void nk_glfw3_shutdown(void)
{
    nk_font_atlas_clear(&glfw.atlas);
    nk_free(&glfw.ctx);
    nk_glfw3_device_destroy();
    memset(&glfw, 0, sizeof(glfw));
}

#endif
#endif
