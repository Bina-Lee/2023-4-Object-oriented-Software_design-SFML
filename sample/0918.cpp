// #include <iostream>
#include <SFML/Graphics.hpp>
using namespace sf;
using namespace std;

int main()
{
    sf::RenderWindow window(sf::VideoMode(500, 500), "SFML works!");
    sf::CircleShape shape(50.f);
    shape.setFillColor(sf::Color::Green);
    float s = 50;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        if (Keyboard::isKeyPressed(Keyboard::Left))
            s--;
        if (Keyboard::isKeyPressed(Keyboard::Right))
            s++;
        shape.setRadius(s);
        // cout << s;
        window.clear();
        window.draw(shape);
        window.display();
    }
    return 0;
}