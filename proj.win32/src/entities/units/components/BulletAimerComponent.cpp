#include "entities/units/components/BulletAimerComponent.h"

#include "assets/Textures.h"
#include "entities/units/components/ComponentBase.h"
#include "entities/units/Unit.h"
#include "entities/units/UnitSpawner.h"
#include "entities/bullets/Bullet.h"
#include "entities/bullets/BulletGroup.h"
#include "gui/GameGUI.h"
#include "input/MouseInput.h"
#include "input/KeyboardInput.h"
#include "map/Cam.h"
#include "states/Game.h"
#include "StateManager.h"
#include "utility/Logger.h"

using namespace cocos2d;

BEGIN_ENTITIES_NS
BEGIN_UNITS_NS
BEGIN_COMPONENTS_NS

//private

//public
void BulletAimerComponent::init() {
    type = UNIT_COMPONENT_TYPE_BULLET_AIMER;

    weapon_sprite = Sprite::create();
    weapon_sprite->setAnchorPoint(Vec2(.5f, .5f));
    weapon_sprite->setVisible(false);
    root::map_layer->addChild(weapon_sprite, 4);

    gui::game::set_power_text(power);
}

BulletAimerComponent::~BulletAimerComponent() {
    cleanup();
}

void BulletAimerComponent::cleanup() {
    if (is_removal_scheduled()) return;

    root::map_layer->removeChild(weapon_sprite);
    schedule_removal();
}

void BulletAimerComponent::switch_weapon(items::Weapon* _weapon) {
    weapon = _weapon;
    weapon_sprite->setTexture(weapon->get_texture());
    weapon_sprite->setScale(weapon->get_scale());
    weapon_sprite->setTextureRect(Rect(0, 0, weapon_sprite->getTexture()->getContentSize().width, 
                                             weapon_sprite->getTexture()->getContentSize().height));
}

void BulletAimerComponent::update() {
    weapon_sprite->setPosition(Vec2(current_unit->base->getPositionX(), current_unit->base->getPositionY()));

    if (aiming || ready_to_fire) {
        gui::game::set_power_text(power);

        if (input::key_down(EventKeyboard::KeyCode::KEY_W)) {
            power += .05f;
            power = clampf(power, MIN_POWER, MAX_POWER);
        }else if (input::key_down(EventKeyboard::KeyCode::KEY_S)) {
            power -= .05f;
            power = clampf(power, MIN_POWER, MAX_POWER);
        }

        if (input::key_pressed(EventKeyboard::KeyCode::KEY_ESCAPE) || input::get_mouse_button_down(MOUSE_BUTTON_RIGHT)) {
            stop_aiming();
        }
    }

    if (ready_to_fire) return;

    if (aiming) {
        float x = (root::scene->screen_size.width) / 2.0f;
        float y = (root::scene->screen_size.height) / 2.0f;
        angle = atan2(y - input::get_mouse_pos().y, input::get_mouse_pos().x - x) * (180 / M_PI);
        weapon_sprite->setRotation(angle + weapon->get_rotation_offset());
        if (weapon->is_weapon_flippable()) {
            weapon_sprite->setFlippedY(angle >= -90 && angle <= 90);
        }

        if (input::get_mouse_button_down(MOUSE_BUTTON_LEFT)) {
            ready_to_fire = true;
        }
    }
}

void BulletAimerComponent::begin_aiming() {
    aiming = true;
    weapon_sprite->setVisible(true);
    ready_to_fire = false;
}

void BulletAimerComponent::stop_aiming() {
    aiming = false;
    weapon_sprite->setVisible(false);
    ready_to_fire = false;
}

void BulletAimerComponent::fire() {
    if (!aiming) return;

    auto g = bullets::create_group(ref);
    auto b = g->create_bullet(ref->base->getPositionX(), ref->base->getPositionY());
    float r = -angle;

    if (weapon == items::weapon_flame_fireworks) {
        b->add_logic_fire_bullet(r, power);
    }else if (weapon == items::weapon_c4) {
        b->add_logic_c4(r, power);
    }

    gui::game::wait_for_bullet(g);
    map::camera::follow_bullet(g);

    cleanup();
}

END_COMPONENTS_NS
END_UNITS_NS
END_ENTITIES_NS