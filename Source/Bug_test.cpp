
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <array>
#include<algorithm>

////////////////////////////////////////////////////////////

// change this to 1 to trigger the bug
#define TRIGGER_THE_BUG 1

int main()
{
    const float screenWidth = 800.0f;
    const float screenHeight = 600.0f;

    sf::RenderWindow window(sf::VideoMode({(unsigned int)screenWidth, (unsigned int)screenHeight}), "Window", sf::Style::Default, sf::ContextSettings(0, 0, 4));
    window.setVerticalSyncEnabled(true);

    sf::Texture beautifulTexture;
    (void)beautifulTexture.loadFromFile("texture.png");
    std::array<sf::RenderTexture, 2> renderTextures {
        sf::RenderTexture(),
        sf::RenderTexture()
    };

    int renderTextureAALevel = 0;
#if TRIGGER_THE_BUG == 1
    renderTextureAALevel = 4;
#endif

    (void)renderTextures[0].create((unsigned int)screenWidth, (unsigned int)screenHeight, sf::ContextSettings(0, 0, renderTextureAALevel));
    (void)renderTextures[1].create((unsigned int)screenWidth, (unsigned int)screenHeight, sf::ContextSettings(0, 0, renderTextureAALevel));

    auto drawToRenderTextures =[&]() {
        renderTextures[0].clear();
        renderTextures[1].clear();

        sf::Sprite sprite(beautifulTexture);
        renderTextures[0].draw(sprite);

        sprite.setColor(sf::Color::Green); // tint it green so we have some visual distinction between the two renderTextures
        renderTextures[1].draw(sprite);
    };

    std::array<sf::VertexArray, 2> vertexArrays;
    vertexArrays[0].setPrimitiveType(sf::PrimitiveType::Triangles);
    vertexArrays[1].setPrimitiveType(sf::PrimitiveType::Triangles);

    auto makeVertexArrays = [&]() {
        vertexArrays[0].clear();
        vertexArrays[1].clear();

        float xCenter = screenWidth / 2;

        // left half of screen
        vertexArrays[0].append(sf::Vertex({0,0}, {0,0}));
        vertexArrays[0].append(sf::Vertex({xCenter,0}, {xCenter,0}));
        vertexArrays[0].append(sf::Vertex({0,screenHeight}, {0,screenHeight}));

        vertexArrays[0].append(sf::Vertex({0,screenHeight}, {0,screenHeight}));
        vertexArrays[0].append(sf::Vertex({xCenter,0}, {xCenter,0}));
        vertexArrays[0].append(sf::Vertex({xCenter,screenHeight}, {xCenter,screenHeight}));

        // right half of screen
        vertexArrays[1].append(sf::Vertex({xCenter,0}, {xCenter,0}));
        vertexArrays[1].append(sf::Vertex({screenWidth,0}, {screenWidth,0}));
        vertexArrays[1].append(sf::Vertex({xCenter,screenHeight}, {xCenter,screenHeight}));

        vertexArrays[1].append(sf::Vertex({xCenter,screenHeight}, {xCenter,screenHeight}));
        vertexArrays[1].append(sf::Vertex({screenWidth,0}, {screenWidth,0}));
        vertexArrays[1].append(sf::Vertex({screenWidth,screenHeight}, {screenWidth,screenHeight}));
    };

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

        makeVertexArrays();
        drawToRenderTextures();

        // NOTE -- The order of display and draw calls are part of the equation.
        // If We call display() on both render textures and then call draws,
        // there is no bug even with the macro defined to 1.

        renderTextures[0].display();
        window.draw(vertexArrays[0], &renderTextures[0].getTexture());

        renderTextures[1].display();
        window.draw(vertexArrays[1], &renderTextures[1].getTexture());

        /*  EG If the following shit were the code rather than the above, it would work
        renderTextures[0].display();
        renderTextures[1].display();

        window.draw(vertexArrays[0], &renderTextures[0].getTexture());
        window.draw(vertexArrays[1], &renderTextures[1].getTexture());
        //*/

        window.display();
    }

    return 0;
}
