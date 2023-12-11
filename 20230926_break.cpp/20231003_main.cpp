#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
// #include<unistd.h>
#include<vector>
#include<iostream>
#include<ctime>

#define windowW 480
#define windowH 600
#define ballW 30
#define ballH 30
#define paddleW 60
#define paddleH 10
#define bombW 30
#define bombH 40

#define paddleInitH 540

#define frameRate 60

using namespace sf;
using namespace std;
////////////////////////////////////////////////////////////////
class Paddle{
public:
    Paddle(){
        Tpaddle.loadFromFile("images/paddle.png");
        Spaddle.setTexture(Tpaddle);
        x=windowW/2-paddleW/2;
        y=paddleInitH;
        Spaddle.setPosition(x,y);
    }

    void update(int movePos){
        x+=movePos;
             if(x<0)  x=0;
        else if(x>windowW-paddleW)x=windowW-paddleW;
        Spaddle.setPosition(x,y);

        return;
    }

    void draw(RenderWindow &window){
        window.draw(Spaddle);
    }

    int x;
    int y;
private:
    Texture Tpaddle;
    Sprite Spaddle;
};
////////////////////////////////////////////////////////////////
class Ball{
public:
    Ball(){
        Tball.loadFromFile("images/ball.png");
        Sball.setTexture(Tball);
        x=windowW/2-ballW/2;
        y=(double)paddleInitH-ballH-1;

        abovePaddle=true;

        moveLR=-1;
        moveUD=-1;
        moveUDSpeed=3.0;

        Sball.setPosition(x,y);

        Bbounce.loadFromFile("sounds/Bounce.wav");
        bounce.setBuffer(Bbounce);
    }

    void update(Paddle &paddle){
        if(x<=0 || x>=windowW-ballW){
            moveLR*=-1;
            bounce.play();
        }
        if(y<=0){
            moveUD=1;
            bounce.play();
        }
        if(y>=paddleInitH-ballH){
            if(abovePaddle && x+ballW>=paddle.x && x<=paddle.x+paddleW){
                moveUD=-1;
                moveUDSpeed=(rand()%5)/10+2.5;
                bounce.play();
            }
            else{
                abovePaddle=false;
                if(x+ballW>=paddle.x && x<=paddle.x+paddleW){
                    moveLR*=-1;
                    bounce.play();
                }
            }
        }
        x+=moveLR*2;
        y+=moveUD*moveUDSpeed;

        if(abovePaddle && y>=paddleInitH-ballH)y=paddleInitH-ballH;

        Sball.setPosition(x,y);

        return;
    }

    bool isDrop(){
        if(y>=windowH-ballH)return true;
        return false;
    }

    void draw(RenderWindow &window){
        window.draw(Sball);
    }
private:
    int x;
    double y;

    int moveLR;
    int moveUD;

    double moveUDSpeed;

    bool abovePaddle;

    Texture Tball;
    Sprite Sball;

    SoundBuffer Bbounce;
    Sound bounce;
};
////////////////////////////////////////////////////////////////
class Bomb{
public:
    Bomb(Sprite S){
        // Tbomb.loadFromFile("images/bomb.png");
        // Sbomb.setTexture(Tbomb);
        Sbomb=S;
        // Sbomb.setTexture(T);

        x=rand()%(windowW-bombW);
        y=0;

        dropSpeed=rand()%5+5;

        Sbomb.setPosition(x,y);
    }

    void update(){
        y+=dropSpeed;
        Sbomb.setPosition(x,y);
        return;
    }

    bool isExplosion(Paddle &paddle){
        if(y+bombH>paddle.y && y<paddle.y+paddleH){
            if(x+bombW>paddle.x && x<paddle.x+paddleW){
                return true;
            }
        }
        return false;
    }

    bool isBottom(){
        if(y>windowH-bombH)return true;
        return false;
    }

    void draw(RenderWindow &window){
        window.draw(Sbomb);
    }
private:
    int x;
    int y;
    int dropSpeed;

    // Texture Tbomb;
    Sprite Sbomb;
};
////////////////////////////////////////////////////////////////
class Progress{
public:
    Progress(int fr){
        currentRound=0;
        finalRound=fr;

        font.loadFromFile("OpenSans-Bold.ttf");
        text1.setFont(font);
        text2.setFont(font);
        text3.setFont(font);
        text4.setFont(font);
    }

    void beforeStartTimeMinus(){
        beforeStartTime--;
    }

    void draw(RenderWindow &window){
        text1.setString(to_string(beforeStartTime));
        text1.setCharacterSize(100);
        text1.setPosition(windowW/2-28,windowH/2-150);
        window.draw(text1);

        text2.setString("lives left "+to_string(finalRound+1-currentRound));
        text2.setCharacterSize(50);
        text2.setPosition(windowW/2-125,windowH/2);
        window.draw(text2);
    }

    void numInit(){
        beforeStartTime=4;
    }

    void drawEnd(RenderWindow &window){
        text3.setString("Game Over");
        text3.setCharacterSize(50);
        text3.setPosition(windowW/2-140,windowH/2-50);
        window.draw(text3);

        text4.setString("want quit, press Q");
        text4.setCharacterSize(20);
        text4.setPosition(windowW/2-90,windowH/2+25);
        window.draw(text4);
    }

    int currentRound;
    int finalRound;
private:
    int beforeStartTime;

    Font font;
    Text text1;
    Text text2;
    Text text3;
    Text text4;
};
////////////////////////////////////////////////////////////////
int main(){
    srand(static_cast<unsigned int>(time(0)));

    RenderWindow window(VideoMode(windowW,windowH),"Game");

    window.setFramerateLimit(frameRate);

    Texture Tbackground;
    Tbackground.loadFromFile("images/background.png");
    Sprite background;
    background.setTexture(Tbackground);

    Texture Tbomb;
    Tbomb.loadFromFile("images/bomb.png");
    Sprite Sbomb;
    Sbomb.setTexture(Tbomb);

    SoundBuffer Bbomb;
    Sound bombSound;
    Bbomb.loadFromFile("sounds/bomb.wav");
    bombSound.setBuffer(Bbomb);

    // Progress progress(3);

    unique_ptr<Progress>progress(new Progress(3));
    
    vector<Bomb>bomb;

    while(progress->currentRound<progress->finalRound){

        progress->currentRound++;

        // Paddle paddle;
        Paddle *paddle=new Paddle;
        // Ball ball;
        Ball *ball=new Ball;
        // Bomb bomb;
        // Bomb *bomb=new Bomb;
        // vector<Bomb>bomb;

        int frameCheck=0;

        progress->numInit();

        for(int i=0;i<frameRate*3;i++){
            Event e;
            while(window.pollEvent(e)){
                if(e.type==Event::Closed){
                    window.close();
                }
            }

            if(i%frameRate==0)progress->beforeStartTimeMinus();

            window.clear();
            window.draw(background);
            paddle->draw(window);
            ball->draw(window);

            progress->draw(window);

            window.display();
        }

        while(window.isOpen()){
            Event e;
            while(window.pollEvent(e)){
                if(e.type==Event::Closed){
                    window.close();
                }
            }

            if(frameCheck==0)bomb.push_back(Bomb(Sbomb));

            if(Keyboard::isKeyPressed(Keyboard::Left ))paddle->update(-5);
            if(Keyboard::isKeyPressed(Keyboard::Right))paddle->update( 5);

            ball->update(*paddle);

            // bomb->update();
            for(int i=0;i<bomb.size();i++)bomb[i].update();

            for(int i=(int)bomb.size()-1;i>=0;i--){
                if(bomb[i].isBottom()){
                    bomb.erase(bomb.begin()+i);
                }
            }

            if(ball->isDrop())break;

            // if(bomb->isExplosion(*paddle))break;
            bool flag=false;
            for(int i=0;i<(int)bomb.size();i++){
                if(bomb[i].isExplosion(*paddle)){
                    flag=true;
                    break;
                }
            }
            if(flag){
                bombSound.play();
                break;
            }

            window.clear();
            window.draw(background);

            paddle->draw(window);
            ball->draw(window);
            // bomb->draw(window);
            for(int i=0;i<(int)bomb.size();i++)bomb[i].draw(window);
            

            window.display();
            // usleep(20000);

            frameCheck=(frameCheck+1)%50;
        }

        delete paddle;
        delete ball;
        // delete bomb
        bomb.clear();

    }

    window.clear();
    window.draw(background);
    progress->drawEnd(window);
    window.display();

    while(1){
        Event e;
        while(window.pollEvent(e)){
            if(e.type==Event::Closed){
                window.close();
            }
        }
  
        if(Keyboard::isKeyPressed(Keyboard::Q))break;
    }

    return 0;
}