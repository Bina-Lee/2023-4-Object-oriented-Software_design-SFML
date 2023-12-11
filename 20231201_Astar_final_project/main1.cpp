#include <iostream> 
#include <chrono>
#include <SFML/Graphics.hpp> 
#include <array>
#include <cfloat>

using namespace std;
using namespace sf;

// namespace를  이용한 상수, 변수  정의 
namespace gbl {
    namespace PATHFINDING {
        constexpr unsigned short CHECKS_PER_FRAME = 78*43;
    }
    namespace SCREEN {
        constexpr unsigned char RESIZE = 1;
        constexpr unsigned short HEIGHT = 700;
        constexpr unsigned short WIDTH = 1280;
        constexpr chrono::microseconds FRAME_DURATION(16667);
    }
    namespace MAP {
        constexpr unsigned char CELL_SIZE = 16;
        constexpr unsigned char ORIGIN_X = 8;
        constexpr unsigned char ORIGIN_Y = 8;
        constexpr unsigned short COLUMNS = 78;
        constexpr unsigned short ROWS = 43;
        enum Cell {
            Empty,
            Invalid,
            Path,
            Visited,
            Wall
        };
    }
    // template 함수 
    //
    // i_value is plus : true - false = 0 
    // i_value is minus: false - true = -1 
    template<typename value_type>
    char sign(const value_type i_value) {
        return (0 < i_value) - (0 > i_value);
    }

    // 2차원 지도 각  셀  상태에 대한 배열
    // (x, y) 형태로 나타내가 위해서 columns를  먼저 지정 
    template<typename value_type = MAP::Cell>
    using Map = std::array<std::array<value_type, MAP::ROWS>, MAP::COLUMNS>;
    // 2차원 좌표 타입
    template<typename value_type = unsigned short>
    using Position = std::pair<value_type, value_type>;
    Position<short> get_mouse_cell(const sf::RenderWindow& i_window) {
        float mouse_x = sf::Mouse::getPosition(i_window).x - MAP::ORIGIN_X * SCREEN::RESIZE;
        float mouse_y = sf::Mouse::getPosition(i_window).y - MAP::ORIGIN_Y * SCREEN::RESIZE;
        return Position<short>(
            floor(mouse_x / MAP::CELL_SIZE / SCREEN::RESIZE),
            floor(mouse_y / MAP::CELL_SIZE / SCREEN::RESIZE));
    }
}

class Astar{
private:
    int current_path_index = 0; // 현재 시각화된 경로의 인덱스

    Texture Tchar;
    Sprite Schar;

public:

    Astar(){
        Tchar.loadFromFile("resource/butterfly.png"); 
        Schar.setTexture(Tchar);
    }

    std::vector<gbl::Position<>> final_path; // 최종 경로를 저장할 벡터

    bool astar_search_completed = false;

    /*
    화면에 셀을 그리는 함수
    */
    void draw_map(const unsigned short _x,
                    const unsigned short _y,
                    const gbl::Position<>& _finish_position,
                    const gbl::Position<>& _start_position,
                    sf::RenderWindow& _window,
                    sf::Sprite& _map_sprite,
                    const gbl::Map<>& _map){
        for (unsigned short a = 0; a < gbl::MAP::COLUMNS; a++){
            for (unsigned short b = 0; b < gbl::MAP::ROWS; b++){
                _map_sprite.setPosition((float)gbl::MAP::ORIGIN_X + _x + a * gbl::MAP::CELL_SIZE,
                    (float)gbl::MAP::ORIGIN_Y + _y + b * gbl::MAP::CELL_SIZE);
                if (_start_position == _finish_position && a == _start_position.first && b == _start_position.second){
                    //_map_sprite.setColor(sf::Color(0, 255, 0)); // 시작점을 녹색으로 표시
                    Schar.setPosition((float)gbl::MAP::ORIGIN_X + _x + a * gbl::MAP::CELL_SIZE,
                        (float)gbl::MAP::ORIGIN_Y + _y + b * gbl::MAP::CELL_SIZE);
                }else

                if (a == _finish_position.first && b == _finish_position.second){
                    _map_sprite.setColor(sf::Color(255, 0, 0));
                }else if (a == _start_position.first && b == _start_position.second){
                    //_map_sprite.setColor(sf::Color(0, 255, 0));
                    Schar.setPosition((float)gbl::MAP::ORIGIN_X + _x + a * gbl::MAP::CELL_SIZE,
                        (float)gbl::MAP::ORIGIN_Y + _y + b * gbl::MAP::CELL_SIZE);
                }else{
                    gbl::MAP::Cell cell_type = _map[a][b];
                    switch (cell_type){
                        case gbl::MAP::Cell::Empty:{
                            _map_sprite.setColor(sf::Color(36, 36, 85));
                            break;}
                        case gbl::MAP::Cell::Path:{
                            _map_sprite.setColor(sf::Color(255, 219, 85));
                            break;}
                        case gbl::MAP::Cell::Visited:{
                            _map_sprite.setColor(sf::Color(0, 36, 255));
                            break;}
                        case gbl::MAP::Cell::Wall:{
                            _map_sprite.setColor(sf::Color(255, 255, 255));}
                    }
                }
                _window.draw(_map_sprite);
                _window.draw(Schar);
            }
        }
    }

    gbl::MAP::Cell get_cell(const gbl::Position<short>& _cell, const gbl::Map<>& _map){
        if (0 <= _cell.first && 0 <= _cell.second && _cell.first < gbl::MAP::COLUMNS && _cell.second <
            gbl::MAP::ROWS){
            return _map[_cell.first][_cell.second];
        }
        return gbl::MAP::Cell::Invalid;
    }
    /*
    _cell에  대하여  인접  cell들의  좌표를  반환
    */
    std::vector<gbl::Position<>> get_adjacent_cells(const gbl::Position<>& _cell, const gbl::Map<>& _map){
        std::vector<gbl::Position<>> adjacent_cells;
        std::vector<gbl::Position<>> diagonal_adjacent_cells;
        for (auto a = -1; a < 2; a++)
        {
            for (auto b = -1; b < 2; b++)
            {
                // 자기자신  제외
                if (0 == a && 0 == b)continue;
                gbl::MAP::Cell _type = get_cell(gbl::Position<short>(_cell.first + a, _cell.second + b), _map);
                // 벽이거나, 범위  밖이  아닌  경우  포함
                if (gbl::MAP::Cell::Invalid != _type && gbl::MAP::Cell::Wall != _type){
                    adjacent_cells.push_back(gbl::Position<>(a + _cell.first, b + _cell.second));
                }
            }
        }
        return adjacent_cells;
    }

    /*
    heuristic score 계산
    */
    float calculate_h_score(const gbl::Position<>& _cell_0, const gbl::Position<>& _cell_1){
        float distance_x = abs(_cell_1.first - _cell_0.first);
        float distance_y = abs(_cell_1.second - _cell_0.second);
        return std::max(distance_x, distance_y) + std::min(distance_x, distance_y) * (sqrt(2) - 1);
    }

    bool astar_search(unsigned short& _path_length,
                        unsigned short& _total_checks,
                        std::chrono::microseconds & _duration,
                        std::map<gbl::Position<>,
                        gbl::Position<>>&_previous_cell,
                        std::vector<gbl::Position<>>&_path_vector,
                        gbl::Map<float>&_f_scores,
                        gbl::Map<float>&_g_scores,
                        const gbl::Map<float>&_h_scores,
                        const gbl::Position<>&_finish_position,
                        const gbl::Position<>&_start_position,
                        gbl::Map<>&_map){
        // 시간 측정
        std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
        // 8번만 A* 반복을 수행하고, frame을  다시 그린다.
        for (unsigned short a = 0; a < gbl::PATHFINDING::CHECKS_PER_FRAME; a++){
            // 알고리즘 종료
            if (1 == _path_vector.empty()){
                _duration += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start_time);
                return true;
            }
            std::vector<gbl::Position<>>::iterator min_f_cell_iterator = _path_vector.begin();
            gbl::Position<> min_f_cell;
            // f score값이 가장  작은 cell을  찾아낸다.
            // 이  부분은 priority queue 등을 이용하여 개선이 가능하다. 
            for (std::vector<gbl::Position<>>::iterator a = 1 + _path_vector.begin(); a != _path_vector.end(); a++){
                if (_f_scores[a->first][a->second] < _f_scores[min_f_cell_iterator->first][min_f_cell_iterator->second]){
                    min_f_cell_iterator = a;
                }
            }
            min_f_cell = *min_f_cell_iterator;

            // 길이 없는 경우
            if (FLT_MAX == _f_scores[min_f_cell.first][min_f_cell.second]){
                _duration += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start_time);
                return true;
            }
            _path_vector.erase(min_f_cell_iterator);
            _map[min_f_cell.first][min_f_cell.second] = gbl::MAP::Cell::Visited;
            _total_checks++;
            // 최종 목적지에 도달한 경우 
            if (min_f_cell == _finish_position){
                gbl::Position<> path_cell = min_f_cell;
                do{
                    _path_length++;
                    _map[path_cell.first][path_cell.second] = gbl::MAP::Cell::Path;
                    path_cell = _previous_cell.at(path_cell);
                } while (_start_position != path_cell);
                _duration += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start_time);
                
                // // 여기에 목적지와 시작점 교환 및 A* 알고리즘 재설정 코드 추가
                // gbl::Position<> temp = _start_position;
                // _start_position = _finish_position;
                // _finish_position = temp;

                // // A* 알고리즘 재시작을 위한 설정
                // astar_reset(astar_finished, astar_path_length, astar_total_checks, astar_duration, astar_previous_cell, astar_path_vector, astar_f_scores, astar_g_scores, astar_h_scores, _finish_position, _start_position, _map);

                // // 경로 탐색을 다시 시작하도록 설정
                // astar_finished = false;
                
                return 1;
            }
            // 주변 셀들에 대해서 업데이트
            for (const gbl::Position<>& adjacent_cell : get_adjacent_cells(min_f_cell, _map)){
                if (gbl::MAP::Cell::Visited != _map[adjacent_cell.first][adjacent_cell.second]){
                    float g_score = _g_scores[min_f_cell.first][min_f_cell.second];
                    if (abs(adjacent_cell.first - min_f_cell.first) == abs(adjacent_cell.second - min_f_cell.second)){
                        // 대각선 거리
                        g_score += sqrt(2);
                    }else{  // 상하좌우 거리
                        g_score++;
                    }
                    // g score (실제  거리)가 더  짧아지는 경우에 대해서만,
                    if (g_score < _g_scores[adjacent_cell.first][adjacent_cell.second]){
                        _previous_cell[adjacent_cell] = min_f_cell;
                        _f_scores[adjacent_cell.first][adjacent_cell.second] = g_score + _h_scores[adjacent_cell.first][adjacent_cell.second];
                        _g_scores[adjacent_cell.first][adjacent_cell.second] = g_score;
                        // 이미  추가되지
                        if (_path_vector.end() == std::find(_path_vector.begin(), _path_vector.end(), adjacent_cell)){
                            _path_vector.push_back(adjacent_cell);
                        }
                    }
                }
            }
        }
        _duration += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start_time);
        return 0;
    }


    /*
    A* 알고리즘 초기화
    */
    void astar_reset(bool& _finished,
                        unsigned short& _path_length,
                        unsigned short& _total_checks,
                        std::chrono::microseconds& _duration,
                        std::map<gbl::Position<>, gbl::Position<>>& _previous_cell,
                        std::vector<gbl::Position<>>& _path_vector,
                        gbl::Map<float>& _f_scores,
                        gbl::Map<float>& _g_scores,
                        gbl::Map<float>& _h_scores,
                        const gbl::Position<>& _finish_position,
                        const gbl::Position<>& _start_position,
                        gbl::Map<>& _map){
        _finished = 0;
        _path_length = 0;
        _total_checks = 0;
        _duration = std::chrono::microseconds(0);
        _previous_cell.clear();
        _path_vector.clear();
        _path_vector.push_back(_start_position);

        for (unsigned short a = 0; a < gbl::MAP::COLUMNS; a++){
            for (unsigned short b = 0; b < gbl::MAP::ROWS; b++){
                if (_map[a][b] != gbl::MAP::Cell::Wall) {
                    _map[a][b] = gbl::MAP::Cell::Empty;
                }

                _h_scores[a][b] = calculate_h_score(gbl::Position<>(a, b),
                    _finish_position);
                if (a == _start_position.first && b == _start_position.second){
                    _f_scores[a][b] = _h_scores[a][b];
                    _g_scores[a][b] = 0;
                }else{
                    _f_scores[a][b] = FLT_MAX;
                    _g_scores[a][b] = FLT_MAX;
                }
            }
        }
    }

    // A* 알고리즘 완료 후 경로 역추적 및 저장
    void save_final_path(const gbl::Position<>& start, const gbl::Position<>& end, const std::map<gbl::Position<>, gbl::Position<>>& came_from) {
        gbl::Position<> current = end;
        while (current != start) {
            final_path.push_back(current);
            current = came_from.at(current);
        }
        final_path.push_back(start); // 시작점도 추가
        std::reverse(final_path.begin(), final_path.end()); // 시작점에서 종점 순으로 뒤집기
    }

    void draw_path(sf::RenderWindow& window, sf::Sprite& sprite) {
        if (current_path_index < final_path.size()) {
            auto& position = final_path[current_path_index];
            sprite.setPosition(position.first * gbl::MAP::CELL_SIZE, position.second * gbl::MAP::CELL_SIZE);
            window.draw(sprite);
            current_path_index++; // 다음 경로 위치로 이동
        }
    }


};

int main() {

    Astar astar;

    bool astar_finished = 0;
    unsigned short astar_path_length = 0;
    unsigned short astar_total_checks = 0;
    // Astar 수행시간 저장
    // == chrono::duration<double, micro> 
    chrono::microseconds astar_duration(0);
    map<gbl::Position<>, gbl::Position<>> astar_previous_cell;
    vector<gbl::Position<>> astar_path_vector;
    gbl::Map<float> astar_h_scores = {};
    gbl::Map<float> astar_g_scores = {};
    gbl::Map<float> astar_f_scores = {};
    //--------- SFML 화면  그리기 ---------------
    bool mouse_pressed = 0;
    bool pause_search = 0;
    chrono::microseconds lag(0);
    chrono::steady_clock::time_point previous_time;
    sf::Event event;
    sf::RenderWindow window(sf::VideoMode(gbl::SCREEN::RESIZE * gbl::SCREEN::WIDTH,
        gbl::SCREEN::RESIZE * gbl::SCREEN::HEIGHT), "Pathfinding by A*", sf::Style::Close);
    window.setView(sf::View(sf::FloatRect(0, 0, gbl::SCREEN::WIDTH, gbl::SCREEN::HEIGHT)));


    window.setFramerateLimit(60);


    sf::Sprite map_sprite;
    //sf::Texture font_texture;
    //font_texture.loadFromFile("Resources/Images/Font.png"); 
    sf::Texture map_texture;
    map_texture.loadFromFile("Astar/Map.png");
    gbl::Map<> map = {};
    gbl::Position<> finish_position;//(gbl::MAP::COLUMNS - 1, gbl::MAP::ROWS - 1);
    gbl::Position<> start_position(0, 0);
    gbl::Position<short> mouse_cell_start;

    bool finish_position_set=false;

    // 모든 셀을  empty상태로 설정
    for (array<gbl::MAP::Cell, gbl::MAP::ROWS>& column : map) {
        column.fill(gbl::MAP::Cell::Empty);
    }
    map_sprite.setTexture(map_texture);
    previous_time = chrono::steady_clock::now();
    while (1 == window.isOpen()) {
        chrono::microseconds delta_time =
            chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - previous_time);
        lag += delta_time;
        previous_time += delta_time;
        // 최소 frame 유지시간 == FRAME_DURATION 
        while (gbl::SCREEN::FRAME_DURATION <= lag){
            // 화면을 다시 그릴 필요가 있는지 검사 
            bool map_updated = 0;
            // 다음 번  연산시간 설정
            lag -= gbl::SCREEN::FRAME_DURATION;
            // 키보드 입력에 대한  검사
            while (1 == window.pollEvent(event)){
                switch (event.type){
                    case sf::Event::Closed:{
                        window.close();
                        break;}
                    case sf::Event::KeyPressed:{
                        switch (event.key.code){
                            case sf::Keyboard::Enter:{ //Pause/Resume
                                pause_search = 1 - pause_search;
                                break;}
                            case sf::Keyboard::R:{ // 검색을 다시 시작 {
                                map_updated = 1;
                                break;}
                            case sf::Keyboard::Space:{ // 맵을  지운다. 
                                for (unsigned short a = 0; a < gbl::MAP::COLUMNS; a++){
                                    for (unsigned short b = 0; b < gbl::MAP::ROWS; b++){
                                        map[a][b] = gbl::MAP::Empty;
                                    }
                                }
                                map_updated = 1;}
                        }
                    }
                    case sf::Event::MouseButtonPressed:{
                        if(start_position==finish_position){
                            if (event.mouseButton.button == sf::Mouse::Left) {
                                // gbl::Position<short> mouse_cell = gbl::get_mouse_cell(window);
                                // if (0 <= mouse_cell.first && 0 <= mouse_cell.second && mouse_cell.first < gbl::MAP::COLUMNS && mouse_cell.second < gbl::MAP::ROWS) {
                                //     // 새로운 목적지 설정
                                //     finish_position = static_cast<gbl::Position<>>(mouse_cell);
                                //     // A* 알고리즘 재시작
                                //     astar.astar_reset(astar_finished, astar_path_length, astar_total_checks, astar_duration, astar_previous_cell, astar_path_vector, astar_f_scores, astar_g_scores, astar_h_scores, finish_position, start_position, map);
                                //     map_updated = 1;  // 지도 업데이트 플래그 설정
                                // }
                                gbl::Position<short> mouse_cell = gbl::get_mouse_cell(window);
                                if (0 <= mouse_cell.first && 0 <= mouse_cell.second && mouse_cell.first < gbl::MAP::COLUMNS && mouse_cell.second < gbl::MAP::ROWS) {
                                    // 새로운 목적지 설정
                                    finish_position = static_cast<gbl::Position<>>(mouse_cell);
                                    // A* 알고리즘 재시작
                                    astar.astar_reset(astar_finished, astar_path_length, astar_total_checks, astar_duration, astar_previous_cell, astar_path_vector, astar_f_scores, astar_g_scores, astar_h_scores, finish_position, start_position, map);
                                    map_updated = 1;  // 지도 업데이트 플래그 설정
                                }
                            }
                        }
                        
                    }
                }
            }

            // drag and drop에  의한 셀  상태  설정
            // 마우스 버튼을 누른  상태에서 움직일 때  셀  상태 변경
            if (true == sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) ||
                true == sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)){
                if (false == mouse_pressed){
                    mouse_pressed = true;
                    mouse_cell_start = gbl::get_mouse_cell(window);
                }
            }else{
                mouse_pressed = 0;
            }
            if (true == mouse_pressed){
                char step_x;
                char step_y;
                unsigned short line_size;
                gbl::Position<short> mouse_cell = gbl::get_mouse_cell(window);
                line_size = 1 + std::max(abs(mouse_cell.first - mouse_cell_start.first), abs(mouse_cell.second -
                mouse_cell_start.second));
                step_x = gbl::sign(mouse_cell.first - mouse_cell_start.first);
                step_y = gbl::sign(mouse_cell.second - mouse_cell_start.second);
                for (unsigned short a = 0; a < line_size; a++){
                    gbl::Position<short> cell;
                    cell.first = mouse_cell_start.first + step_x * floor(a * (1 + abs(mouse_cell.first -
                        mouse_cell_start.first)) / static_cast<float>(line_size));
                    cell.second = mouse_cell_start.second + step_y * floor(a * (1 + abs(mouse_cell.second -
                        mouse_cell_start.second)) / static_cast<float>(line_size));
                    if (0 <= cell.first && 0 <= cell.second && cell.first < gbl::MAP::COLUMNS && cell.second <
                        gbl::MAP::ROWS){
                        if (finish_position != static_cast<gbl::Position<>>(cell) && start_position !=
                            static_cast<gbl::Position<>>(cell)){
                            if (1 == sf::Mouse::isButtonPressed(sf::Mouse::Left)){
                                if (gbl::MAP::Cell::Wall != map[cell.first][cell.second]){
                                    map_updated = 1;
                                    if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::F)){
                                        // F키를 누른 상태에서 마우스 클릭하면 종료 지점 변경 
                                        map[cell.first][cell.second] = gbl::MAP::Cell::Empty;
                                        finish_position = cell;
                                    }else if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
                                        // S키를 누른 상태에서 마우스를 클릭하면 종료 지점 변경 
                                        map[cell.first][cell.second] = gbl::MAP::Cell::Empty;
                                        start_position = cell;
                                    }else{
                                        // 나머지 경우는 모두  벽으로 지정
                                        map[cell.first][cell.second] = gbl::MAP::Cell::Wall;
                                    }
                                }
                            }else if (1 == sf::Mouse::isButtonPressed(sf::Mouse::Right)){
                                // 오른쪽 버튼을 누르면 벽을 지울 수  있음
                                if (gbl::MAP::Cell::Wall == map[cell.first][cell.second]){
                                    map_updated = 1;
                                    map[cell.first][cell.second] = gbl::MAP::Cell::Empty;
                                }
                            }
                        }
                    }
                }
                mouse_cell_start = mouse_cell;
            }

            // if (1 == map_updated){
            //     astar.astar_reset(astar_finished,
            //                 astar_path_length,
            //                 astar_total_checks,
            //                 astar_duration,
            //                 astar_previous_cell,
            //                 astar_path_vector,
            //                 astar_f_scores,
            //                 astar_g_scores,
            //                 astar_h_scores,
            //                 finish_position,
            //                 start_position,
            //                 map);
            //     if (astar_finished && !astar.final_path.empty()) {
            //         // 시작점을 경로상의 다음 픽셀로 이동
            //         start_position = astar.final_path[1];

            //         map_updated=1;
                    
            //         if(start_position==finish_position)map_updated = 0;  // 다음 프레임에서 다시 A* 알고리즘을 실행하기 위해 플래그 초기화
            //     }
            // }
            // if (false == pause_search){
            //     if (false == astar_finished){
            //     astar_finished = astar.astar_search(
            //                         astar_path_length,
            //                         astar_total_checks,
            //                         astar_duration,
            //                         astar_previous_cell,
            //                         astar_path_vector,
            //                         astar_f_scores,
            //                         astar_g_scores,
            //                         astar_h_scores,
            //                         finish_position,
            //                         start_position,
            //                         map);
            //     }
            // }

            if (false == pause_search){

                if (start_position == finish_position) {
                    ;
                }
                else if (false == astar_finished){
                    // A* 알고리즘 실행
                    astar_finished = astar.astar_search(
                                        astar_path_length,
                                        astar_total_checks,
                                        astar_duration,
                                        astar_previous_cell,
                                        astar_path_vector,
                                        astar_f_scores,
                                        astar_g_scores,
                                        astar_h_scores,
                                        finish_position,
                                        start_position,
                                        map);
                }

                if (astar_finished) {
                    // 최종 경로 저장
                    astar.save_final_path(start_position, finish_position, astar_previous_cell);

                    if (!astar.final_path.empty()) {
                        // 시작점을 경로상의 다음 픽셀로 이동
                        start_position = astar.final_path[1];
                        // A* 알고리즘 재설정
                        astar_finished = false; // A* 알고리즘 재시작을 위해 플래그 재설정
                        astar.astar_reset(astar_finished, 
                                            astar_path_length, 
                                            astar_total_checks, 
                                            astar_duration, 
                                            astar_previous_cell, 
                                            astar_path_vector, 
                                            astar_f_scores, 
                                            astar_g_scores, 
                                            astar_h_scores, 
                                            finish_position, 
                                            start_position, 
                                            map);
                    }
                }
            }

            // 화면 다시  그리기 
            window.clear();
            astar.draw_map(0, 0,
                finish_position,
                start_position,
                window,
                map_sprite,
                map);

            window.display();
        }
    }
    return 0;
}