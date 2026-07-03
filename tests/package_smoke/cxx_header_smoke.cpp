#include <prompteditor/example.h>

int main() {
    int value = 0;
    return prompteditor_add_checked(1, 2, &value) && value == 3 ? 0 : 1;
}
