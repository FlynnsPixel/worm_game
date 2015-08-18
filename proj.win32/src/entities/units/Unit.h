#ifndef _UNIT_H_
#define _UNIT_H_

/*
this class temporarily handles all player movement and other things, will be modified in future
*/

#include <vector>
#include <2d/CCSprite.h>
#include <physics/CCPhysicsContact.h>

#include "entities/EntityDefines.h"
#include "entities/units/components/ComponentBase.h"

BEGIN_ENTITIES_NS
BEGIN_UNITS_NS

namespace cc = cocos2d;

enum UnitType {

    UNIT_TYPE_CAPTAIN,
    UNIT_TYPE_MINION
};

//forward declares begin
namespace components {

    struct PlayerMoveComponent;
};
struct UnitTeam;
//forward declares end

class Unit {

    public:
        Unit();
        ~Unit();

        cc::Sprite* base;
        cc::PhysicsBody* pbody;

        bool player_input = false;
        UnitType type;
        UnitTeam* team;

        void update();
        void despawn();

        void take_damage(float amount);

        void schedule_removal() { removal_scheduled = true; }
        bool is_scheduled_removal() { return removal_scheduled; }

        template <typename T> T*    add_component();
        template <typename T> void  remove_component();
        template <typename T> T*    get_component();

    private:
        bool removal_scheduled = false;
        std::vector<components::ComponentBase*> components;
        float damage = 0.0f;

        bool on_contact_run(cc::PhysicsContact&);
        //void on_post_contact(cc::PhysicsContact&, const cc::PhysicsContactPostSolve&);
};

END_UNITS_NS
END_ENTITIES_NS

#endif