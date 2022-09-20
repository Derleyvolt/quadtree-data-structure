#include <SFML/Graphics.hpp>

// g++ -c A.cpp -IC:/Users/Derley/Downloads/SFML-2.5.1-windows-gcc-7.3.0-mingw-32-bit/SFML-2.5.1/include && 
// g++ A.o -o A -LC:/Users/Derley/Downloads/SFML-2.5.1-windows-gcc-7.3.0-mingw-32-bit/SFML-2.5.1/lib -lsfml-graphics -lsfml-window -lsfml-system && ./A
// export LD_LIBRARY_PATH=C:/Users/Derley/Downloads/SFML-2.5.1-windows-gcc-7.3.0-mingw-32-bit/SFML-2.5.1/lib && ./A

// g++ -o A A.cpp -LC:/Users/Derley/Downloads/SFML-2.5.1-windows-gcc-7.3.0-mingw-32-bit/SFML-2.5.1/lib -lsfml-graphics -lsfml-window -lsfml-system && ./A

#include <iostream>
#include <string>
#include <deque>

using namespace std;

void draw_square(sf::Vector2f pos, sf::Vector2f size, sf::RenderWindow& wnd, sf::Color color) {
    sf::Vertex square[] = {
        sf::Vertex(pos),
        sf::Vertex({pos.x + size.y, pos.y}),
        sf::Vertex({pos.x + size.y, pos.y + size.x}),
        sf::Vertex({pos.x, pos.y + size.x}),
        sf::Vertex(pos),
    };

    wnd.draw(square, 5, sf::LinesStrip);
}

pair<sf::Vector2f, sf::Vector2i> circle_to_square(sf::Vector2f pos, int r) {
    sf::Vector2f p(pos.x - r, pos.y - r);
    return {p, {2*r, 2*r}};
}

class Box {
public:
    Box() = default;

    Box(sf::Vector2f ltop, int h, int w) : top_left(ltop), w(w), h(h) {
    }

    // colision dot
    bool colision(sf::Vector2f p) {
        return p.x >= top_left.x && p.x <= top_left.x + w &&
               p.y >= top_left.y && p.y <= top_left.y + h;
    }

    // colision rectangle
    bool colision(sf::Vector2f p, sf::Vector2i size) {
        return colision(p) || colision(sf::Vector2f(p.x + size.y, p.y)) ||
                              colision(sf::Vector2f(p.x, p.y + size.x)) ||
                              colision(sf::Vector2f(p.x + size.y, p.y + size.x));
    }

    sf::Vector2f top_left;
    int h, w;
};

class Quadtree {
public:
    Quadtree(Box region, sf::RenderWindow& wind, int nivel = 0) : region(region), window(wind), nivel(nivel) {
        is_leaf = 1;
    }

    #define MAX_OBJECTS 10

    int insert(sf::Vector2f p, int r) {
        auto sq = circle_to_square(p, r);
        
        if(is_leaf) {
            data.push_back({p, r});

            // precisamos dividir o nó atual
            if(data.size() == MAX_OBJECTS && this->nivel < 6) {
                is_leaf = 0;

                for(int i = 0; i < 4; i++) {
                    quadtree[i] = new Quadtree(get_region(region, i), window, this->nivel + 1);

                    for(int j = 0; j < data.size(); j++) {
                        auto aux = circle_to_square(data[j].first, data[j].second);
                        if(quadtree[i]->region.colision(aux.first, aux.second)) {
                            quadtree[i]->insert(data[j].first, data[j].second);
                        }
                    }
                }

                data.clear();
            }

            return 1;
        } else {
            int ans = 0;
            for(int i = 0; i < 4; i++) {
                if(quadtree[i]->region.colision(sq.first, sq.second)) {
                    ans = ans | quadtree[i]->insert(p, r);
                }
            }

            return ans;
        }
    }

    // retorna uma região (primeiro, segundo, terceiro ou quarto quadrante) dependendo do valor de idx (varia de 0 a 3)
    Box get_region(Box reg, int idx) {
        sf::Vector2f p = sf::Vector2f{reg.top_left.x, reg.top_left.y};
        int w = reg.w;
        int h = reg.h;

        if(idx == 0) {
            return Box(p, h/2, w/2);
        } else if(idx == 1) {
            p.x = p.x + w/2;
            return Box(p, h/2, w/2);
        } else if(idx == 2) {
            p.y = p.y + h/2;
            return Box(p, h/2, w/2);
        } else {
            p.y = p.y + h/2;
            p.x = p.x + w/2;
            return Box(p, h/2, w/2);
        }
    }   

    void draw() {
        sf::Vector2f pos = sf::Vector2f(region.top_left.x, this->region.top_left.y);

        auto mouse_pos = mouse.getPosition(window);

        draw_square(pos, sf::Vector2f(region.h, region.w), window, sf::Color::White);

        sf::CircleShape circle;

        if(!is_leaf) {
            for(int i = 0; i < 4; i++) {
                this->quadtree[i]->draw();
            }
        }

        for(auto a : data) {
            circle.setFillColor(sf::Color::Green);
            circle.setRadius(a.second);
            circle.setPosition(a.first);
            window.draw(circle);
            draw_square(a.first, sf::Vector2f(2*a.second, 2*a.second), window, sf::Color::White);
        }
    }

private:
    Quadtree*                       quadtree[4];
    Box                             region;
    deque<pair<sf::Vector2f, int>>  data;

    int                is_leaf;
    sf::RenderWindow&  window;
    sf::Mouse          mouse;
    int                nivel; // limitar o número de divisões/quadrantes evita problema com estouro de pilha
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 800), "SFML works!");
    //sf::CircleShape shape(100.f);
    //sf::RectangleShape shape;
    //shape.setFillColor(sf::Color::Red);

    // shape.setSize({50, 50});

    // shape.setPosition({0, 0});

    srand(time(NULL));

    Quadtree quadt(Box({0, 0}, 800, 800), window);

    // for(int i = 0; i < 87; i++) {
    //     quadt.insert(sf::Vector2f(rand() % 800, rand() % 800), rand() % 20);
    // }

    sf::Mouse mouse;

    window.setFramerateLimit(60);

    int flag = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        //window.clear();

        quadt.draw();

        static int count;

        if(mouse.sf::Mouse::isButtonPressed(sf::Mouse::Left) && flag == 0) {
            flag = 1;
            count++;
            auto mouse_pos = mouse.getPosition(window);
            quadt.insert(sf::Vector2f(mouse_pos.x, mouse_pos.y), 10);
            cout << count << endl;
        } else if(!mouse.sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            flag = 0;
        }

        //window.draw(shape);
        //draw_square({1,1}, {200, 200}, window);
        window.display();
    }

    return 0;
}
