#include <kinc/system.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/threads/thread.h>
#include <kinc/threads/event.h>
#include <string.h>

extern int krafix_compile(const char *source, char *output, int *length, const char *targetlang,
                          const char *system, const char *shadertype);

static int compile_shader(char *output, const char *source, int *length,
                              kinc_g4_shader_type_t type) {
#ifdef KINC_WINDOWS
	const char *system = "windows";
#elif defined(KINC_MACOS)
	const char *system = "osx";
#elif defined(KINC_LINUX)
	const char *system = "linux";
#elif defined(KINC_ANDROID)
	const char *system = "android";
#elif defined(KINC_IOS)
    const char *system = "ios";
#else
	const char *system = "unknown";
#endif

#ifdef KORE_VULKAN
	const char *targetlang = "spirv";
#elif defined(KORE_DIRECT3D11) || defined(KORE_DIRECT3D12)
	const char *targetlang = "d3d11";
#elif defined(KORE_DIRECT3D9) || defined(KORE_DIRECT3D12)
	const char *targetlang = "d3d9";
#elif defined(KORE_METAL)
	const char *targetlang = "metal";
#elif defined(KORE_OPENGL)
#if !defined(KINC_ANDROID) && !defined(KORE_EMSCRIPTEN)
	const char *targetlang = "glsl";
#else
	const char *targetlang = "essl";
#endif
#endif

	int errors = krafix_compile(source, output, length, targetlang, system,
	                            type == KINC_G4_SHADER_TYPE_FRAGMENT ? "frag" : "vert");
	return errors;
}

#define VERT_SRC \
"#version 450\n"\
"in vec3 vertexPosition;\n"\
"in vec4 vertexColor;\n"\
"uniform mat4 projectionMatrix;\n"\
"out vec4 fragmentColor;\n"\
"void main() {\n"\
"	gl_Position = projectionMatrix * vec4(vertexPosition, 1.0);\n"\
"	fragmentColor = vertexColor;\n"\
"}\n"

#define FRAG_SRC \
"#version 450\n"\
"in vec4 fragmentColor;\n"\
"out vec4 FragColor;\n"\
"void main() {\n"\
"	FragColor = fragmentColor;\n"\
"}\n"


static void compile_worker(void *data) {
	kinc_event_t *evt = (kinc_event_t *)data;
	kinc_event_wait(evt);
	char buffer[4096];
    memset(buffer, 0, 4096);
    int length;
	compile_shader(buffer, VERT_SRC, &length, KINC_G4_SHADER_TYPE_VERTEX);
    memset(buffer, 0, 4096);
	compile_shader(buffer, FRAG_SRC, &length, KINC_G4_SHADER_TYPE_FRAGMENT);
}

static kinc_event_t event;

void update(void *data) {
    kinc_g4_begin(0);
	kinc_event_signal(&event);
    kinc_g4_end(0);
    kinc_g4_swap_buffers();
	kinc_stop();
}

int kickstart(int argc, char **argv) {
    kinc_init("mackrafixrepro", 1024, 768, NULL, NULL);
	kinc_threads_init();
	kinc_event_init(&event, false);
	kinc_thread_t t;
	kinc_thread_init(&t, compile_worker, &event);
    kinc_set_update_callback(update, NULL);
    kinc_start();
	kinc_threads_quit();
    return 0;
}
