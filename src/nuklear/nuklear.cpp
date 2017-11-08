#include "../gl3w.h"

#define NK_IMPLEMENTATION
#include "../NuklearAndConfig.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

#include "nuklear_glfw_gl3.h"
#include "../Globals.h"

#ifndef NK_GLFW_DOUBLE_CLICK_LO
#define NK_GLFW_DOUBLE_CLICK_LO 0.02
#endif
#ifndef NK_GLFW_DOUBLE_CLICK_HI
#define NK_GLFW_DOUBLE_CLICK_HI 0.2
#endif

#define NK_CHECK_FLAG(a, flags) (((a & flags) != 0))

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

	struct nk_glfw_device *dev = &g->glfw.ogl;
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
	//Create cursors
	{
		g->glfw.cursors[NK_CURSOR_RESIZE_HORIZONTAL] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		g->glfw.cursors[NK_CURSOR_RESIZE_VERTICAL] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		g->glfw.cursors[NK_CURSOR_TEXT] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		g->glfw.cursors[NK_CURSOR_ARROW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		g->glfw.activeCursor = NK_CURSOR_ARROW;
	}
}

NK_INTERN void
nk_glfw3_device_upload_atlas(const void *image, int width, int height)
{
	struct nk_glfw_device *dev = &g->glfw.ogl;
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
	struct nk_glfw_device *dev = &g->glfw.ogl;
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
	if (g->glfw.ctx.style.cursor_type != g->glfw.activeCursor) {
		g->glfw.activeCursor = g->glfw.ctx.style.cursor_type;
		glfwSetCursor(g->glfw.win, g->glfw.cursors[g->glfw.activeCursor]);
	}

	struct nk_glfw_device *dev = &g->glfw.ogl;
	GLfloat ortho[4][4] = {
		{ 2.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f,-2.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f,-1.0f, 0.0f },
		{ -1.0f,1.0f, 0.0f, 1.0f },
	};
	ortho[0][0] /= (GLfloat)g->glfw.width;
	ortho[1][1] /= (GLfloat)g->glfw.height;

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
	glViewport(0, 0, (GLsizei)g->glfw.display_width, (GLsizei)g->glfw.display_height);
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
				{ NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, position) },
				{ NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, uv) },
				{ NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_glfw_vertex, col) },
				{ NK_VERTEX_LAYOUT_END }
			};
			memset(&config, 0, sizeof(config));
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
			nk_convert(&g->glfw.ctx, &dev->cmds, &vbuf, &ebuf, &config); }
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

		/* iterate over and execute each draw command */
		nk_draw_foreach(cmd, &g->glfw.ctx, &dev->cmds)
		{
			if (!cmd->elem_count) continue;
			glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
			glScissor(
				(GLint)(cmd->clip_rect.x * g->glfw.fb_scale.x),
				(GLint)((g->glfw.height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * g->glfw.fb_scale.y),
				(GLint)(cmd->clip_rect.w * g->glfw.fb_scale.x),
				(GLint)(cmd->clip_rect.h * g->glfw.fb_scale.y));
			glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
			offset += cmd->elem_count;
		}
		nk_clear(&g->glfw.ctx);
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
	//For CTRL+S and CTRL+F, which are not integrated into the rest of the input system, yet.
	if (mods & GLFW_MOD_CONTROL) {
		g->glfw.controlKeyCB(key, action);
	}
	if (action == GLFW_RELEASE) {
		return;
	}
	for (int i = 0; i < g->glfw.numKeys; i++) {
		if (key != keymap[i].glfw_key) { continue; }
		if (NK_CHECK_FLAG(keymap[i].flags, KEY_FLAG_CHECK_CTRL)) {
			if (NK_CHECK_FLAG(keymap[i].flags, KEY_FLAG_CTRL) != NK_CHECK_FLAG(mods, GLFW_MOD_CONTROL)) {
				continue;
			}
		}
		if (action == GLFW_REPEAT && !NK_CHECK_FLAG(keymap[i].flags, KEY_FLAG_REPEATS)) { continue; }

		//It passed all the checks!
		g->glfw.activatedKeys[i] = true;
	}

}

NK_API void
nk_glfw3_char_callback(GLFWwindow *win, unsigned int codepoint)
{
	(void)win;
	if (g->glfw.text_len < NK_GLFW_TEXT_MAX)
		g->glfw.text[g->glfw.text_len++] = codepoint;
}

NK_API void
nk_gflw3_scroll_callback(GLFWwindow *win, double xoff, double yoff)
{
	(void)win; (void)xoff;
	g->glfw.scroll.x += (float)xoff;
	g->glfw.scroll.y += (float)yoff;
}

NK_API void
nk_glfw3_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	double x, y;
	if (button != GLFW_MOUSE_BUTTON_LEFT) return;
	glfwGetCursorPos(window, &x, &y);
	if (action == GLFW_PRESS) {
		double dt = glfwGetTime() - g->glfw.last_button_click;
		if (dt > NK_GLFW_DOUBLE_CLICK_LO && dt < NK_GLFW_DOUBLE_CLICK_HI)
		{
			//nk_input_button(&g->glfw.ctx, NK_BUTTON_DOUBLE, (int)x, (int)y, nk_true);
			g->glfw.doubleClickDown = nk_true;
			g->glfw.doubleClickPos = nk_vec2(x, y);
		}
		g->glfw.last_button_click = glfwGetTime();
	}
	else {
		//nk_input_button(&g->glfw.ctx, NK_BUTTON_DOUBLE, (int)x, (int)y, nk_false);
		g->glfw.doubleClickDown = nk_false;
	}
}

NK_INTERN void
nk_glfw3_my_clipbard_copy(nk_handle usr, TextLine* lines, int numLines, TextCursor start, TextCursor end)
{
	char *str = 0;
	(void)usr;
	int len = 0;
	int startByte = nk_my_bytes_to_glyph(&lines[start.line], start.col);
	int endByte = nk_my_bytes_to_glyph(&lines[end.line], end.col);
	if (start.line == end.line) {
		len = endByte - startByte;
		str = (char*)malloc((size_t)len + 1);
		if (!str) return;
		memcpy(str, &lines[start.line].text[startByte], (size_t)len);
		str[len] = '\0';
		glfwSetClipboardString(g->glfw.win, str);
		free(str);
	}
	else {
		len = lines[start.line].text.len - startByte;
		len++;//For the \n
		for (int i = start.line + 1; i < end.line; i++) {
			len += lines[i].text.len;
			len++; //For the \n
		}
		len += endByte;

		str = (char*)malloc((size_t)len + 1);
		if (!str) return;

		int offset = 0;
		int copyLen = lines[start.line].text.len - startByte;
		memcpy(str, &lines[start.line].text[startByte], copyLen);
		offset += copyLen;
		str[offset] = '\n';
		offset++;
		for (int i = start.line + 1; i < end.line; i++) {
			copyLen = lines[i].text.len;
			printf("Stuff: %s\n", lines[i].text.data);
			memcpy(&str[offset], lines[i].text.data, copyLen);
			offset += copyLen;
			str[offset] = '\n';
			offset++;
		}
		copyLen = endByte;
		memcpy(str + offset, lines[end.line].text.data, copyLen);
		offset += copyLen;
		str[offset] = '\0';
		glfwSetClipboardString(g->glfw.win, str);
		free(str);
	}

}

NK_INTERN void
nk_glfw3_my_clipbard_paste(nk_handle usr, struct nk_my_text_edit *edit)
{
	const char *text = glfwGetClipboardString(g->glfw.win);
	if (text) nk_my_textedit_paste(edit, text, nk_strlen(text));
	(void)usr;
}

NK_INTERN void
nk_glfw3_clipbard_paste(nk_handle usr, struct nk_text_edit *edit)
{
	const char *text = glfwGetClipboardString(g->glfw.win);
	if (text) nk_textedit_paste(edit, text, nk_strlen(text));
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
	glfwSetClipboardString(g->glfw.win, str);
	free(str);
}

NK_API struct nk_context*
nk_glfw3_init(GLFWwindow *win, enum nk_glfw_init_state init_state, ControlKeyCallback controlKeyCB)
{
	g->glfw.win = win;
	if (init_state == NK_GLFW3_INSTALL_CALLBACKS) {
		glfwSetScrollCallback(win, nk_gflw3_scroll_callback);
		glfwSetCharCallback(win, nk_glfw3_char_callback);
		glfwSetMouseButtonCallback(win, nk_glfw3_mouse_button_callback);
		glfwSetKeyCallback(win, nk_glfw3_key_callback);
	}
	nk_init_default(&g->glfw.ctx, 0);
	g->glfw.ctx.clip.copy = nk_glfw3_clipbard_copy;
	g->glfw.ctx.clip.paste = nk_glfw3_clipbard_paste;
	g->glfw.ctx.clip.userdata = nk_handle_ptr(0);
	g->glfw.last_button_click = 0;
	g->glfw.ctx.my_clip.copy = nk_glfw3_my_clipbard_copy;
	g->glfw.ctx.my_clip.paste = nk_glfw3_my_clipbard_paste;
	g->glfw.ctx.my_clip.userdata = nk_handle_ptr(0);

	g->glfw.controlKeyCB = controlKeyCB;

	g->glfw.doubleClickDown = false;
	g->glfw.doubleClickPos = nk_vec2(0, 0);

	g->glfw.numKeys = sizeof(keymap) / sizeof(nk_glfw_keymapping);
	g->glfw.activatedKeys = new bool[g->glfw.numKeys];
	for (int i = 0; i < g->glfw.numKeys; i++) {
		g->glfw.activatedKeys[i] = false;
	}

	nk_glfw3_device_create();
	return &g->glfw.ctx;
}

NK_API void
nk_glfw3_font_stash_begin(struct nk_font_atlas **atlas)
{
	nk_font_atlas_init_default(&g->glfw.atlas);
	nk_font_atlas_begin(&g->glfw.atlas);
	*atlas = &g->glfw.atlas;
}

NK_API void
nk_glfw3_font_stash_end(void)
{
	const void *image; int w, h;
	image = nk_font_atlas_bake(&g->glfw.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
	nk_glfw3_device_upload_atlas(image, w, h);
	nk_font_atlas_end(&g->glfw.atlas, nk_handle_id((int)g->glfw.ogl.font_tex), &g->glfw.ogl.null);
	if (g->glfw.atlas.default_font)
		nk_style_set_font(&g->glfw.ctx, &g->glfw.atlas.default_font->handle);
}

NK_API void
nk_glfw3_new_frame(void)
{
	int i;
	double x, y;
	struct nk_context *ctx = &g->glfw.ctx;
	struct GLFWwindow *win = g->glfw.win;

	glfwGetWindowSize(win, &g->glfw.width, &g->glfw.height);
	glfwGetFramebufferSize(win, &g->glfw.display_width, &g->glfw.display_height);
	g->glfw.fb_scale.x = (float)g->glfw.display_width / (float)g->glfw.width;
	g->glfw.fb_scale.y = (float)g->glfw.display_height / (float)g->glfw.height;

	nk_input_begin(ctx);
	for (i = 0; i < g->glfw.text_len; ++i)
		nk_input_unicode(ctx, g->glfw.text[i]);


#if NK_GLFW_GL3_MOUSE_GRABBING
	/* optional grabbing behavior */
	if (ctx->input.mouse.grab)
		glfwSetInputMode(g->glfw.win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	else if (ctx->input.mouse.ungrab)
		glfwSetInputMode(g->glfw.win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#endif

	for (int i = 0; i < g->glfw.numKeys; i++) {
		if (g->glfw.activatedKeys[i]) {
			nk_input_key(ctx, keymap[i].nk_key, true);
			ctx->input.keyboard.keys[keymap[i].nk_key].clicked++;
		}
		g->glfw.activatedKeys[i] = false;
	}

	nk_input_key(ctx, NK_KEY_SHIFT, glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) || glfwGetKey(win, GLFW_KEY_LEFT_SHIFT));

	glfwGetCursorPos(win, &x, &y);
	nk_input_motion(ctx, (int)x, (int)y);
#if NK_GLFW_GL3_MOUSE_GRABBING
	if (ctx->input.mouse.grabbed) {
		glfwSetCursorPos(g->glfw.win, ctx->input.mouse.prev.x, ctx->input.mouse.prev.y);
		ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
		ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
	}
#endif
	nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
	nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
	nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
	nk_input_button(&g->glfw.ctx, NK_BUTTON_DOUBLE, g->glfw.doubleClickPos.x, g->glfw.doubleClickPos.y, g->glfw.doubleClickDown);
	nk_input_scroll(ctx, g->glfw.scroll);
	nk_input_end(&g->glfw.ctx);
	g->glfw.text_len = 0;
	g->glfw.scroll = nk_vec2(0, 0);
}

NK_API
void nk_glfw3_shutdown(void)
{
	delete[] g->glfw.activatedKeys;
	nk_font_atlas_clear(&g->glfw.atlas);
	nk_free(&g->glfw.ctx);
	nk_glfw3_device_destroy();
	memset(&g->glfw, 0, sizeof(g->glfw));
}

NK_API
bool nk_glfw3_load_image(const char* path, struct nk_image* image) {
	int w, h, numChannels;
	unsigned char* data = stbi_load(path, &w, &h, &numChannels, STBI_rgb_alpha);
	if (data == NULL) {
		return false;
	}

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);

	*image = nk_image_id(tex);
	image->w = w;
	image->h = h;
	image->region[0] = 0;
	image->region[1] = 0;
	image->region[2] = w;
	image->region[3] = h;
	return true;
}
