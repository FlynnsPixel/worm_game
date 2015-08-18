#include "entities/units/Unit.h"

#include "assets/Textures.h"
#include "debug/Logger.h"
#include "entities/bullets/Bullet.h"
#include "entities/units/UnitSpawner.h"
#include "entities/units/components/PlayerMoveComponent.h"
#include "entities/units/components/BulletAimerComponent.h"
#include "entities/units/components/ColliderComponent.h"
#include "physics/Physics.h"
#include "states/Game.h"
#include "StateManager.h"

BEGIN_ENTITIES_NS
BEGIN_UNITS_NS

using namespace cocos2d;

//public
Unit::Unit() {
    base = Sprite::createWithTexture(assets::textures::duck);
    base->setScale(.5f);
    root::map_layer->addChild(base, 1);

    PhysicsMaterial mat;
    mat.density = 0.0f;
    mat.friction = 40.0f;
    mat.restitution = .5f;
    pbody = PhysicsBody::createBox(Size(base->getContentSize().width * base->getScaleX(),
                                        base->getContentSize().height * base->getScaleY()), mat);
    pbody->setCollisionBitmask(1);
    pbody->setContactTestBitmask(true);
    pbody->setRotationEnable(false);
    //pbody->setPositionOffset(Vec2(0, -10));
    //pbody->setGravityEnable(false);
    base->setPhysicsBody(pbody);

    physics::add_on_contact_run(CC_CALLBACK_1(Unit::on_contact_run, this), this);

    add_component<components::ColliderComponent>()->init();
}

bool Unit::on_contact_run(PhysicsContact& contact) {
    auto a = contact.getShapeA()->getBody()->getNode();
    auto b = contact.getShapeB()->getBody()->getNode();

    if (a && b) {
        if (a == base || b == base) {
            //loop through all component on_contact_run functions (if one exists)
            //if a collision occurred, true is returned
            bool collided = false;
            for (auto& c : components) {
                if (c->on_contact_run(contact)) collided = true;
            }
            if (collided) return true;
        }
    }

    return false;
}

Unit::~Unit() {
    pbody->release();
    root::map_layer->removeChild(base);
    physics::remove_on_contact_run(this);
}

void Unit::update() {
    for (int n = 0; n < components.size(); ++n) {
        components[n]->update();
    }
}

void Unit::take_damage(float amount) {
    damage -= amount;
}

//-- begin template definitions --

template <typename T> T* Unit::add_component() {
    T* t = new T(this);
    components.push_back(t);
    return t;
}
template components::PlayerMoveComponent*       Unit::add_component<components::PlayerMoveComponent>();
template components::BulletAimerComponent*      Unit::add_component<components::BulletAimerComponent>();
template components::ColliderComponent*         Unit::add_component<components::ColliderComponent>();

template <typename T> T* Unit::get_component() {
    for (auto* c : components) {
        if (typeid(*c) == typeid(T)) {
            return (T*)c;
        }
    }
    return NULL;
}
template components::PlayerMoveComponent*       Unit::get_component<components::PlayerMoveComponent>();
template components::BulletAimerComponent*      Unit::get_component<components::BulletAimerComponent>();
template components::ColliderComponent*         Unit::get_component<components::ColliderComponent>();

template <typename T> void Unit::remove_component() {
    for (auto* c : components) {

    }
}
template void Unit::remove_component<components::PlayerMoveComponent>();
template void Unit::remove_component<components::BulletAimerComponent>();
template void Unit::remove_component<components::ColliderComponent>();

//-- end component template definitions --

END_UNITS_NS
END_ENTITIES_NS
