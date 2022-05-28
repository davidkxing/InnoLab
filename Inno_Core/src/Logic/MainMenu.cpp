#include "MainMenu.h"

int selectionvar = 50;

MainMenu::MainMenu(float tempX, float tempY, float tempW, float tempH) {
    x = tempX;  //x pixels from the walls
    y = tempY;  //height/2 = middle of the window
    w = tempW;  //width 
    h = tempH;  //height
}

void MainMenu::displayButtons(uint8_t* frame_buf, int screen_width, int screen_height) {
    for (int height = y - 50 - h / 2; height <= y - 50 + h / 2; height++) {
        for (int width = x - w / 2; width <= x + w / 2; width++) {
            if ((width >= 0 && width < screen_width && height >= 0 && height < screen_height)) {
                int curr_pixel = height * screen_width * 4 + width * 4;
                frame_buf[curr_pixel] = 0;//blue
                frame_buf[++curr_pixel] = 255;//green
                frame_buf[++curr_pixel] = 0; //red
            }
        }
    }

    for (int height = y + 50 - h / 2; height <= y + 50 + h / 2; height++) {
        for (int width = x - w / 2; width <= x + w / 2; width++) {
            if ((width >= 0 && width < screen_width && height >= 0 && height < screen_height)) {
                int curr_pixel = height * screen_width * 4 + width * 4;
                frame_buf[curr_pixel] = 0;//blue
                frame_buf[++curr_pixel] = 0;//green
                frame_buf[++curr_pixel] = 255; //red
            }
        }
    }
}

void MainMenu::displaySelection(uint8_t* frame_buf, int screen_width, int screen_height) {
    if (key_up) {
        selectionvar = 50;
    }
    else if (key_down) {
        selectionvar = -50;
    }

    for (int height = y - selectionvar - h / 2; height <= y - selectionvar + h / 2; height++) {
        for (int width = x - 100 - w / 2; width <= x - 100 + w / 2; width++) {
            if ((width >= 0 && width < screen_width && height >= 0 && height < screen_height)) {
                int curr_pixel = height * screen_width * 4 + width * 4;
                frame_buf[curr_pixel] = 200;//blue
                frame_buf[++curr_pixel] = 200;//green
                frame_buf[++curr_pixel] = 200; //red
            }
        }
    }
}

bool MainMenu::checkState(bool selection) {
    if (selection && selectionvar == 50) {
        printf("checkstate true\n");
        return true;
    }
    else if (selection && selectionvar == -50) {
        printf("checkstate false\n");
        close_game();
    }
}

void MainMenu::close_game() {
    retro_unload_game();
    retro_deinit();
    exit(1);
}