#include "entities/Bullet.h"

#include <physics/CCPhysicsBody.h>
#include <base/CCEventDispatcher.h>
#include "cocos2d.h"

#include "assets/Textures.h"
#include "assets/Particles.h"
#include "debug/Logger.h"
#include "SceneManager.h"
#include "StateManager.h"
#include "states/Game.h"

using namespace cocos2d;

BEGIN_ENTITIES_NS
BEGIN_BULLET_NS

//public

//private
std::vector<BulletPtr> bullets;
EventListenerPhysicsContact* contact_listener;

struct BulletDataBase {

	int timer = 0;
	std::function<void()> update_callback = nullptr;

	virtual ~BulletDataBase() {

	}
};

struct BulletDataTest : public BulletDataBase {

	bool gen_explosion = false;

	virtual ~BulletDataTest() {

	}
};

void init() {
    contact_listener = EventListenerPhysicsContact::create();
    contact_listener->onContactBegin = physics_contact;
    root::scene->getEventDispatcher()->addEventListenerWithFixedPriority(contact_listener, 10);
}

void deinit() {
    root::scene->getEventDispatcher()->removeEventListener(contact_listener);
}

BulletPtr create_bullet(int x, int y) {
    BulletPtr b(new Bullet(x, y));
    bullets.push_back(b);
    return b;
}

void update() {
    for (int n = 0; n < bullets.size(); ++n) {
        bullets[n]->update();
        if (bullets[n]->is_removal_scheduled()) {
            bullets.erase(bullets.begin() + n, bullets.begin() + n + 1);
            --n;
        }
    }
}

bool physics_contact(PhysicsContact& contact) {
    auto a = contact.getShapeA()->getBody()->getNode();
    auto b = contact.getShapeB()->getBody()->getNode();

    if (a && b) {
        for (int n = 0; n < bullets.size(); ++n) {
            if ((a == bullets[n]->base && b == states::game::terrain->base) || 
                (b == bullets[n]->base && a == states::game::terrain->base)) {
                bullets[n]->physics_contact(contact);
                return false;
            }
        }
    }

    return true;
}

//-- begin Bullet class --

Bullet::Bullet(int x, int y) {
    base = Sprite::createWithTexture(assets::textures::test_bullet);
    base->setPosition(x, y);
    root::scene->addChild(base, 1);

    pbody = PhysicsBody::createBox(Size(base->getContentSize().width * base->getScaleX(),
                                        base->getContentSize().height * base->getScaleY()));
    pbody->setCollisionBitmask(1);
    pbody->setContactTestBitmask(true);
    pbody->setRotationEnable(false);
    base->setPhysicsBody(pbody);
}

Bullet::~Bullet() {
    cleanup();
}

void Bullet::cleanup() {
    root::scene->removeChild(base, 1);
    for (int n = 0; n < data_types.size(); ++n) {
        delete data_types[n];
    }
    data_types.clear();
}

void Bullet::schedule_removal() {
    to_be_removed = true;
}

bool Bullet::physics_contact(PhysicsContact& contact) {
    auto a = contact.getShapeA()->getBody()->getNode();
    auto b = contact.getShapeB()->getBody()->getNode();

    if (a && b && (a == states::game::terrain->base || b == states::game::terrain->base)) {
		auto bullet_explosion = ParticleSystemQuad::create(assets::particles::bullet_explosion_name);
        bullet_explosion->setPosition(base->getPosition());
        bullet_explosion->setScale(.325f);
        root::scene->addChild(bullet_explosion, 1);

        schedule_removal();
    }

    return true;
}

void Bullet::update() {
    for (int n = 0; n < data_types.size(); ++n) {
        if (data_types[n]->update_callback != nullptr) data_types[n]->update_callback();
    }
}

void Bullet::add_btype_test(float angle, float power) {
    type = BULLET_TYPE_TEST;
    float force_x = cos(angle / (180.0f / M_PI)) * 100000.0f * power;
    float force_y = sin(angle / (180.0f / M_PI)) * 100000.0f * power;
    pbody->applyImpulse(Vec2(force_x, force_y));
    
    BulletDataTest* data = new BulletDataTest();
    data_types.push_back(data);
    data->update_callback = [data, this]() {
        if (data->gen_explosion) return;
		if (++data->timer >= 40) {
			data->gen_explosion = true;
            for (int n = 0; n < 8; ++n) {
                auto b = create_bullet(base->getPositionX(), base->getPositionY());
                float angle = ((rand() / (float)RAND_MAX) * 90.0f) + 45.0f;
                b->add_btype_test2(angle, ((rand() / (float)RAND_MAX) * .2f) + .5f);
            }
            auto bullet_explosion = ParticleSystemQuad::create("Ring.plist");
            bullet_explosion->setPosition(base->getPosition());
            bullet_explosion->setScale(.8f);
            root::scene->addChild(bullet_explosion, 1);
        }
    };
}

void Bullet::add_btype_test2(float angle, float power) {
    type = BULLET_TYPE_TEST2;
    float force_x = cos(angle / (180.0f / M_PI)) * 100000.0f * power;
    float force_y = sin(angle / (180.0f / M_PI)) * 100000.0f * power;
	pbody->applyImpulse(Vec2(force_x, force_y));
}

//-- end Bullet class --

END_BULLET_NS
END_ENTITIES_NS