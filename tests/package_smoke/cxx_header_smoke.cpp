#include <prompteditor/example.h>

int main() {
    int value = 0;
    return pp_add_checked(1, 2, &value) && value == 3 ? 0 : 1;
}
