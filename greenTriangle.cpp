#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <vector>
#include <string>

void ShaderLog(unsigned int shader)
{
    int infologLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
    if (infologLen > 1)
    {
        int charsWritten = 0;
        std::vector<char> infoLog(infologLen);
        glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog.data());
        std::cout << "InfoLog: " << infoLog.data() << std::endl;
    }
}

void checkGLError(const std::string& operation)
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cout << "OpenGL error during " << operation << ": " << error << std::endl;
    }
}

int main()
{
    sf::Window window(sf::VideoMode({ 800, 600 }), "OpenGL Triangle");
    window.setVerticalSyncEnabled(true);

    window.setActive(true);
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cout << "Failed to initialize GLEW: " << glewGetErrorString(glewError) << std::endl;
        return -1;
    }

    // 1

    // Вершинный шейдер
    const char* vertexShaderSource =
        "#version 330 core\n"
        "layout (location = 0) in vec2 position;\n"
        "void main() {\n"
        "    gl_Position = vec4(position, 0.0, 1.0);\n"
        "}\n";

    // Фрагментный шейдер  
    const char* fragmentShaderSource =
        "#version 330 core\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    fragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "}\n";

    // вершинный шейдер
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    ShaderLog(vertexShader);
    checkGLError("vertex shader compilation");

    // фрагментный шейдер
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    ShaderLog(fragmentShader);
    checkGLError("fragment shader compilation");

    // 2
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkGLError("shader program linking");

    // 3
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    if (posAttrib == -1) {
        std::cout << "Error: attribute 'position' not found in shader program" << std::endl;
        return -1;
    }
    std::cout << "Position attribute location: " << posAttrib << std::endl;

    // 4
    float vertices[] = {
         0.0f,  0.5f,  // Верхняя вершина
        -0.5f, -0.5f,  // Левая нижняя
         0.5f, -0.5f   // Правая нижняя
    };

    GLuint VBO;
    glGenBuffers(1, &VBO);
    checkGLError("VBO generation");

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    checkGLError("VBO data upload");

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    checkGLError("VBO unbind");

    while (window.isOpen())
    {
        // Обработка событий для SFML 3 - ИСПРАВЛЕННЫЙ СИНТАКСИС
        for (std::optional<sf::Event> event = window.pollEvent(); event.has_value(); event = window.pollEvent())
        {
            // Проверяем тип события
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            // Обработка изменения размера окна
            else if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                glViewport(0, 0, resized->size.x, resized->size.y);
            }
        }

        // Очистка буфера цвета (черный фон)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 5

        glUseProgram(shaderProgram);
        checkGLError("use program");

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        checkGLError("bind VBO");

        glEnableVertexAttribArray(posAttrib);
        checkGLError("enable vertex attrib array");

        glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        checkGLError("vertex attrib pointer");

        // Рисуем треугольник
        glDrawArrays(GL_TRIANGLES, 0, 3);
        checkGLError("draw arrays");

        glDisableVertexAttribArray(posAttrib);
        checkGLError("disable vertex attrib array");

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        checkGLError("unbind VBO");

        window.display();
    }

    // 6.

    glDeleteProgram(shaderProgram);

    // Удаление шейдеров
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glDeleteBuffers(1, &VBO);

    return 0;
}
