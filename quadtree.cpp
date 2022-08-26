#include <iostream>
#include <string>
#include <deque>

using namespace std;

struct Point {
    int x, y;
};

class Box {
public:
    Box() = default;

    Box(Point ltop, Point rbottom) : top_left(ltop), bottom_right(rbottom) {

    }

    bool colision(Point p) {
        return p.x >= top_left.x && p.x <= bottom_right.x && p.y <= top_left.y && p.y >= bottom_right.y;
    }

    Point top_left, bottom_right;
};

void print_box(Box region) {
    printf("top-left: (%d, %d)     bottom-right:  (%d, %d) \n\n", region.top_left.x, region.top_left.y,
                                                                  region.bottom_right.x, region.bottom_right.y);
}

class Quadtree {
public:
    Quadtree(Box region) : region(region) {
        is_leaf = 1;
    }

    #define MAX_POINTS 10


    void insert(Point p) {
        if(is_leaf) {
            if(region.colision(p)) {
                data.push_back(p);

                // precisamos dividir o nó atual
                if(data.size() == MAX_POINTS) {
                    is_leaf = 0;

                    for(int i = 0; i < 4; i++) {
                        quadtree[i] = new Quadtree(get_region(region, i));

                        for(int j = 0; j < data.size();) {
                            if(quadtree[i]->region.colision(data[j])) {
                                quadtree[i]->data.push_back(data[j]);
                                data.pop_front();
                            } else {
                                data.push_back(data.front());
                                data.pop_front();
                                j++;
                            }
                        }
                    }

                    data.clear();
                }
            }
        } else {
            for(int i = 0; i < 4; i++) {
                if(quadtree[i]->region.colision(p)) {
                    quadtree[i]->insert(p);
                }
            }
        }
    }

    void check_colision(Point p) {

    }

    // retorna uma região (primeiro, segundo, terceiro ou quarto quadrante) dependendo do valor de idx (varia de 0 a 3)
    Box get_region(Box region, int idx) {
        Point l = region.top_left;
        Point r = region.bottom_right;

        int hori = r.x-l.x;
        int vert = l.y-r.y;

        // esquerda superior
        if(idx == 0) {
            return Box(l, { l.x + hori/2, r.y + vert/2 });
        // direito superior
        } else if(idx == 1) {
            return Box({ l.x + hori/2, l.y }, { r.x, r.y + vert/2 });
        // esquerdo inferior
        } else if(idx == 2) {
            return Box({ l.x, l.y - vert/2 }, { r.x - hori/2, r.y });
        // direito inferor
        } else {
            return Box({ l.x + hori/2, l.y - vert/2 }, r);
        }
    }   

    Quadtree*     quadtree[4];
    Box           region;
private:
    deque<Point>  data;
    int           is_leaf;
};

int main() {
    Quadtree qt(Box({0, 100}, {100, 0}));

    srand(time(NULL));

    for(int i = 0; i < 100; i++) {
        qt.insert(Point{rand() % 100, rand() % 100});
    }

    print_box(qt.quadtree[0]->region);
    return 0;
}
