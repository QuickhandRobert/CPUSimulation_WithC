#include <stdio.h>
#include <conio.h>  // For _getch()

#define MAX_LEN 256

int main() {
    char buffer[MAX_LEN] = "Salam Dash";
    int cursor = 0;
    int len = 0;

    while (1) {
        int ch = _getch();

        // Handle arrow keys (first char is 0 or 224)
        if (ch == 0 || ch == 224) {
            int arrow = _getch();
            switch (arrow) {
                case 75: // Left arrow
                    if (cursor > 0) {
                        cursor--;
                        printf("\b");
                    }
                    break;
                case 77: // Right arrow
                    if (cursor < len) {
                        printf("%c", buffer[cursor]);
                        cursor++;
                    }
                    break;
            }
        } else if (ch == 8) { // Backspace
            if (cursor > 0) {
                for (int i = cursor - 1; i < len - 1; i++)
                    buffer[i] = buffer[i + 1];
                buffer[--len] = '\0';
                cursor--;
                printf("\b \b"); // erase character
                for (int i = cursor; i < len; i++)
                    printf("%c", buffer[i]);
                printf(" ");
                for (int i = cursor; i <= len; i++)
                    printf("\b");
            }
        } else if (ch == '\r') { // Enter
            break;
        } else {
            if (len < MAX_LEN - 1) {
                // Insert character
                for (int i = len; i > cursor; i--)
                    buffer[i] = buffer[i - 1];
                buffer[cursor] = (char)ch;
                buffer[++len] = '\0';

                for (int i = cursor; i < len; i++)
                    printf("%c", buffer[i]);
                for (int i = cursor + 1; i < len; i++)
                    printf("\b");
                cursor++;
            }
        }
    }

    printf("\nFinal buffer: %s\n", buffer);
    return 0;
}
