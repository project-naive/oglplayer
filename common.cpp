#include "Objects.h"

#include <iostream>

void APIENTRY GLdebug(GLenum source​, GLenum type​, GLuint id​,
   GLenum severity​, GLsizei length​, const GLchar* message​, const void* userParam​)
{
	if (id​ == 131154 && severity​ == GL_DEBUG_SEVERITY_MEDIUM && type​ == GL_DEBUG_TYPE_PERFORMANCE)++num;
	std::cout << message​ << '\n';
}

void initContext()
{
	glewInit();
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GLdebug, nullptr);
}