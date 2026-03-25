#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Glyph.hpp>
#include <GL/glu.h>

#include <iostream>
#include <optional>

void initialiseOpenGL(){
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClearDepth(1.f);
    glClearColor(0.f,0.f,0.f,1.f);

    glShadeModel(GL_FLAT);
    
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}


sf::ContextSettings renderSettings;

int main() {
    std::cout<<"PixiModel\n";
    renderSettings.depthBits = 24;
    renderSettings.stencilBits = 8;
    renderSettings.antiAliasingLevel = 0;
    renderSettings.majorVersion = 2;
    renderSettings.minorVersion = 1;
    sf::RenderWindow window(sf::VideoMode({800,600}),"PixiModel",sf::Style::Default,sf::State::Windowed,renderSettings);
    initialiseOpenGL();
    float angle = 0.f;
    while(window.isOpen()){
        while(const std::optional event = window.pollEvent()){
            if(event->is<sf::Event::Closed>())
                window.close();
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)){
            window.close();
        }

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        float aspect = 800.f/600.f;
        gluPerspective(70.f,aspect,0.1f,100.f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glTranslatef(0.f,0.f,-5.f);
        angle += 0.1f;
        glRotatef(angle,0.1f,1.f,0.f);

        glBegin(GL_QUADS);

        //Front
        glColor3f(1,0,0);
        glVertex3f(-1,-1,1);
        glVertex3f(1,-1,1);
        glVertex3f(1,1,1);
        glVertex3f(-1,1,1);

        //Back
        glColor3f(0,1,0);
        glVertex3f(-1,-1,-1);
        glVertex3f(-1,1,-1);
        glVertex3f(1,1,-1);
        glVertex3f(1,-1,-1);

        //Left
        glColor3f(0,0,1);
        glVertex3f(-1,-1,-1);
        glVertex3f(-1,-1,1);
        glVertex3f(-1,1,1);
        glVertex3f(-1,1,-1);

        //Right
        glColor3f(1,1,0);
        glVertex3f(1,-1,-1);
        glVertex3f(1, 1,-1);
        glVertex3f(1, 1, 1);
        glVertex3f(1,-1, 1);

        // Top
        glColor3f(1,0,1);
        glVertex3f(-1,1,-1);
        glVertex3f(-1,1, 1);
        glVertex3f( 1,1, 1);
        glVertex3f( 1,1,-1);

        // Bottom
        glColor3f(0,1,1);
        glVertex3f(-1,-1,-1);
        glVertex3f( 1,-1,-1);
        glVertex3f( 1,-1, 1);
        glVertex3f(-1,-1, 1);

        glEnd();
    

        window.display();
    }
    return 0;
}