#if 0

#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)){
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(); 
        window.draw(shape); 
        window.display();
    }
    return 0;
}

#endif

#if 01

#include <SFML/Graphics.hpp> 
#include <SFML/Audio.hpp>
// #include <windows.h> 
#include<unistd.h>
#include <string>
#include <vector>
#include <iostream> 

using namespace sf; 
using namespace std;

class LunarLander {
private:
    double x, y;                    // 현재 위치 
    double velocity;                // 속도
    double fuel;                    // 연료
    string status;                  // 현재 상태
    Texture t1, t2;                 // 텍스처 객체
    Sprite spaceship, burst;        // 스프라이트 객체
    Font font;                      // 폰트 객체
    Text text;                      // 텍스트 객체
public:
    SoundBuffer Bbomb;
    SoundBuffer BmoveLR;
    SoundBuffer Bengine;

    Sound bomb;
    Sound moveLR;
    Sound engine;

    LunarLander(double h, double v, double f){
        x = 300;
        y = h;
        velocity = v;
        fuel = f; 
        t1.loadFromFile("images/spaceship.png"); 
        t2.loadFromFile("images/burst.png"); 
        spaceship.setTexture(t1); 
        burst.setTexture(t2); 
        spaceship.setPosition(x, y); 
        // burst.setPosition(x + 20, y + 50);
        burst.setPosition(x, y);
        if (!font.loadFromFile("OpenSans-Bold.ttf")) {
            cout << "폰트 파일을 오픈할 수 없음!" << endl; 
        } 
        text.setFont(font);

        Bbomb.loadFromFile("sounds/bomb.wav");
        BmoveLR.loadFromFile("sounds/moveLR.wav");
        Bengine.loadFromFile("sounds/rocket_engine.wav");

        bomb.setBuffer(Bbomb);
        moveLR.setBuffer(BmoveLR);
        engine.setBuffer(Bengine);
    }   // 생성자

    bool checkLanded(){
        if (y <= 0)return true;
        return false;
    }   // 착륙했는지를 검사하는 함수. 만약 높이가 0보다 작으면 착륙한 것이다. 

    void update(double amount){
        if (fuel <= 0) {
            fuel = 0;
            amount = 0;
        }
        fuel = fuel - amount;
        velocity = velocity - amount + 0.8;
        y = y + velocity;
        if (y > 400) {
            y = 400;
            if(bomb.getStatus()==bomb.Stopped && velocity>10){
                bomb.play();
                fuel=0;
            }
            velocity=0;
        }

        if ((!Keyboard::isKeyPressed(Keyboard::Left)) 
         && (!Keyboard::isKeyPressed(Keyboard::Right)))moveLR.stop();
        if (Keyboard::isKeyPressed(Keyboard::Left)){
            x=x-3;
            moveLR.play();
        }
        if (Keyboard::isKeyPressed(Keyboard::Right)){
            x=x+3;
            moveLR.play();
        }

        spaceship.setPosition(x, y);
        // burst.setPosition(x + 20, y + 50);
        burst.setPosition(x, y);
        status = "Press up key!\nheight: " 
                + to_string(400-y) 
                + "\nspeed: " 
                + to_string(velocity) 
                + "\nfuel: " 
                + to_string(fuel);
    }   // 게임 상태를 업데이트한다.

    void draw(RenderWindow &window,bool isburst){
        window.draw(spaceship); 
        if(isburst && fuel!=0){
            window.draw(burst); 
            engine.play();
        }else engine.stop();
        text.setString(status); 
        text.setCharacterSize(20); 
        text.setPosition(10, 100); 
        window.draw(text);
    }   // 화면에 착륙선과 불꽃, 현재 상태를 그린다.
};

int main()
{
    RenderWindow window(VideoMode(600, 480), "LUNAR LANDER");

    window.setFramerateLimit(60);
    Texture t;
    Sprite background; 
    t.loadFromFile("images/background.png"); 
    background.setTexture(t);
    LunarLander lander(300.0, 1.0, 100.0);// 착륙선 객체 생성

    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed)
                window.close();
        }

        bool isburst=false;

        if (Keyboard::isKeyPressed(Keyboard::Up)){
            lander.update(3.0);
            isburst=true;
        }
        else lander.update(0.0);

        window.clear(); 
        window.draw(background); 
        lander.draw(window,isburst);
        window.display(); 

        // Sleep(100);
        usleep(100000);
    }
    return 0; 
}

#endif