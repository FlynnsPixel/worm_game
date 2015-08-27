#include "states/Menu.h"

#include <2d/CCSprite.h>

#include "assets/Textures.h"
#include "gui/Button.h"
#include "network/message/Message.h"
#include "network/message/Stream.h"
#include "network/message/MID.h"
#include "network/server/ServerConnection.h"
#include "network/sockets/Socket.h"
#include "StateManager.h"
#include "utility/General.h"

BEGIN_STATES_NS

namespace menu {

    using namespace network;
    using namespace root;
    using namespace cocos2d;

    //private
    Sprite* main_menu;
    Sprite* options_menu;
    Sprite* customisations_menu;
    Sprite* corner_box_left;
    Sprite* corner_box_right;
    Label* username_label;
    Label* gold_label;
    gui::ButtonPtr lbutton;
    gui::ButtonPtr rbutton;
    gui::ButtonPtr begin_button;
    Node* menu_node;

    bool scrolling = false;
    float scroll_dest_x = 0;

    void create_menu(Sprite*& s, Texture2D& t) {
        s = Sprite::createWithTexture(&t);
        s->setScaleX(scene->screen_size.width / s->getContentSize().width);
        s->setScaleY(scene->screen_size.height / s->getContentSize().height);
        s->setAnchorPoint(Vec2(0, 0));
        menu_node->addChild(s, 1);
    }

    void create_corner_box(Sprite*& s) {
        s = Sprite::createWithTexture(assets::textures::ui_corner_box);
        s->setScaleX(225.0f / s->getContentSize().width);
        s->setScaleY(70.0f / s->getContentSize().height);
        s->setAnchorPoint(Vec2(0, 0));
        s->setPositionY(scene->screen_size.height - (s->getContentSize().height * s->getScaleY()));
        ui_layer->addChild(s, 1);
    }

    void create_corner_label(Label*& l) {
        l = Label::createWithBMFont("fonts/felt.fnt", "n/a", TextHAlignment::CENTER);
        l->setAnchorPoint(Vec2(0, 0));
        l->setVerticalAlignment(TextVAlignment::CENTER);
        ui_layer->addChild(l, 1);
    }

    void scroll_left() {
        if (scrolling) return;

        scrolling = true;
        scroll_dest_x += scene->screen_size.width;
    }

    void scroll_right() {
        if (scrolling) return;

        scrolling = true;
        scroll_dest_x -= scene->screen_size.width;
    }

    //public

    void create_state(State state) {
        switch (state) {
        case STATE_MENU:
            menu_node = Node::create();
            ui_layer->addChild(menu_node, 1);

            create_menu(main_menu, *assets::textures::main_screen);
            create_menu(options_menu, *assets::textures::options_screen);
            create_menu(customisations_menu, *assets::textures::customisation_screen);

            options_menu->setPositionX(-scene->screen_size.width);
            customisations_menu->setPositionX(scene->screen_size.width);

            lbutton = gui::create_button(40, scene->screen_size.height / 2.0f);
            lbutton->set_on_click_callback(scroll_left);
            lbutton->set_idle_texture(assets::textures::arrow_button);
            lbutton->set_size(assets::textures::arrow_button->getContentSize());
            ui_layer->addChild(lbutton->base, 1);

            rbutton = gui::create_button(scene->screen_size.width - 40, scene->screen_size.height / 2.0f);
            rbutton->set_on_click_callback(scroll_right);
            rbutton->set_idle_texture(assets::textures::arrow_button);
            rbutton->set_size(assets::textures::arrow_button->getContentSize());
            rbutton->base->setFlippedX(true);
            ui_layer->addChild(rbutton->base, 1);

            create_corner_box(corner_box_left);
            create_corner_box(corner_box_right);
            int corner_width = corner_box_right->getContentSize().width * corner_box_right->getScaleX();
            int corner_height = corner_box_right->getContentSize().height * corner_box_right->getScaleY();
            corner_box_right->setPositionX(scene->screen_size.width - corner_width);
            corner_box_right->setFlippedX(true);

            create_corner_label(username_label);
            username_label->setWidth(corner_width - 40);
            username_label->setHeight(corner_height);
            username_label->setPositionY(scene->screen_size.height - corner_height);
            create_corner_label(gold_label);
            gold_label->setPositionX(corner_box_right->getPositionX() + 40);
            gold_label->setWidth(corner_width - 40);
            gold_label->setHeight(corner_height);
            gold_label->setPositionY(scene->screen_size.height - corner_height);

            begin_button = gui::create_button(scene->screen_size.width / 2.0f, scene->screen_size.height / 2.0f);
            begin_button->set_idle_texture(assets::textures::begin_button);
            begin_button->set_scale_x(500.0f / begin_button->get_idle_texture()->getContentSize().width);
            begin_button->set_scale_y(250.0f / begin_button->get_idle_texture()->getContentSize().height);
            begin_button->set_size(assets::textures::begin_button->getContentSize());
            begin_button->set_on_click_callback([]() {
                switch_state(STATE_GAME);
            });
            menu_node->addChild(begin_button->base, 1);

            server::tcp_sock.add_message_handler(msg::MID_RECV_MY_ACCOUNT_DETAILS, [](msg::Message* m) {
                if (m->get<msg::GeneralResult>(0) == msg::GENERAL_RESULT_SUCCESS) {
                    char* username = m->get<char*>(1);
                    int gold = m->get<int>(2);
                    username_label->setString(username);
                    gold_label->setString(sstream_cstr(gold));
                }
            });

            msg::send(server::tcp_sock, msg::Stream() << msg::MID_SEND_REQUEST_FOR_MY_ACCOUNT_DETAILS);

            break;
        }
    }

    void remove_state(State state) {
        switch (state) {
        case STATE_MENU:
            ui_layer->removeChild(menu_node);

            ui_layer->removeChild(lbutton->base);
            ui_layer->removeChild(rbutton->base);

            ui_layer->removeChild(corner_box_left);
            ui_layer->removeChild(corner_box_right);
            ui_layer->removeChild(username_label);
            ui_layer->removeChild(gold_label);

            break;
        }
    }

    void update_state(State state) {
        switch (state) {
        case STATE_MENU:
            if (scrolling) {
                float x = menu_node->getPositionX() - (menu_node->getPositionX() - scroll_dest_x) / 8.0f;
                menu_node->setPosition(x, menu_node->getPositionY());
                if (x >= scroll_dest_x - 4.0f && x <= scroll_dest_x + 4.0f) {
                    scrolling = false;
                    menu_node->setPositionX(scroll_dest_x);

                    for (auto c : menu_node->getChildren()) {
                        if (scroll_dest_x + c->getPositionX() < -scene->screen_size.width * 1.5f) {
                            c->setPositionX(c->getPositionX() + (scene->screen_size.width * 3.0f));
                        }else if (scroll_dest_x + c->getPositionX() > scene->screen_size.width * 1.5f) {
                            c->setPositionX(c->getPositionX() - (scene->screen_size.width * 3.0f));
                        }
                    }
                }
            }

            break;
        }
    }
};

END_STATES_NS
