#include <prompteditor/example.h>

int main(void) {
    int value = 0;

    if (!prompteditor_add_checked(4, 5, &value)) {
        return 1;
    }

    return value == 9 ? 0 : 1;
}
