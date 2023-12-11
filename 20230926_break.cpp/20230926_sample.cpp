#include <SFML/Graphics.hpp>
#include <vector>
#include <time.h>
using namespace sf;

int main(int argc, char const *argv[])
{
    srand(time(NULL));

    RenderWindow window(VideoMode(600, 480), "ARKANOID");
    window.setFramerateLimit(60);

    Texture t1, t2, t3, t4;
    t1.loadFromFile("images/block.png");
    t2.loadFromFile("images/background.png");
    t3.loadFromFile("images/ball.png");
    t4.loadFromFile("images/paddle.png");

    Sprite background(t2), ball(t3), paddle(t4);
    paddle.setPosition(300, 460);

    const int size = 100;
    std::vector<Sprite> blocks(100);
    int n = 0;
    auto bsize = t1.getSize();

    for (int i = 1; i <= 10; i++)
    {
        for (int j = 1; j <= 10; j++)
        {
            blocks[n].setTexture(t1);
            blocks[n].setPosition(i * bsize.x, j * bsize.y);
            n++;
        }
    }

    float dx = 3, dy = 3;
    while (window.isOpen())
    {
        Event e;
        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();
        }

        auto ball_pos = ball.getPosition();

        ball_pos.x += dx;
        for (int i = 0; i < blocks.size(); i++)
        {
            if (FloatRect(ball_pos.x + 3, ball_pos.y + 3, 6, 6).intersects(blocks[i].getGlobalBounds()))
            {
                blocks.erase(blocks.begin() + i);
                dx = -dx;
            }
        }
        ball_pos.y += dy;
        for (int i = 0; i < blocks.size(); i++)
        {
            if (FloatRect(ball_pos.x + 3, ball_pos.y + 3, 6, 6).intersects(blocks[i].getGlobalBounds()))
            {
                blocks.erase(blocks.begin() + i);
                dy = -dy;
            }
        }

        if (ball_pos.x < 0 || ball_pos.x > 520)
            dx = -dx;
        if (ball_pos.y < 0 || ball_pos.y > 450)
            dy = -dy;
        if (Keyboard::isKeyPressed(Keyboard::Right))
            paddle.move(5, 0);
        if (Keyboard::isKeyPressed(Keyboard::Left))
            paddle.move(-5, 0);

        if (FloatRect(ball_pos.x, ball_pos.y, 12, 12).intersects(paddle.getGlobalBounds()))
            dy = -(rand() % 5 + 2);

        ball.setPosition(ball_pos.x, ball_pos.y);

        window.clear();
        window.draw(background);
        window.draw(ball);
        window.draw(paddle);

        for (auto &obj : blocks)
            window.draw(obj);

        window.display();
    }

    return 0;
}
