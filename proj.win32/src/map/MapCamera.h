#ifndef _MAP_CAMERA_H_
#define _MAP_CAMERA_H_

#include <2d/CCCamera.h>

namespace map {

    namespace cc = cocos2d;

    class MapCamera {

        public:
            MapCamera();

            cc::Camera* cam;

            void update();
    };
};

#endif