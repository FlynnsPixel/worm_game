#include "SimpleInput.h"

const int input::last_key_code = (int)cc::EventKeyboard::KeyCode::KEY_ENTER;
bool input::keys[last_key_code];
cc::EventListenerKeyboard* input::kb_event;

void input::init() {
    kb_event = cc::EventListenerKeyboard::create();

    kb_event->onKeyPressed = [](cc::EventKeyboard::KeyCode key_code, cc::Event* event) {
        CCLOG("key pressed: %d", (int)key_code);
        if ((int)key_code >= 0 && (int)key_code <= last_key_code) {
            keys[(int)key_code] = true;
        }
    };
    
    state::scene->getEventDispatcher()->addEventListenerWithFixedPriority(input::kb_event, 10);
}

void input::update() {
}