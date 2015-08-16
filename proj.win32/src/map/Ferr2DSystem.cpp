#include "map/Ferr2DSystem.h"

#include <2d/CCSprite.h>
#include <base/CCDirector.h>
#include <physics/CCPhysicsBody.h>
#include <renderer/CCTextureCache.h>
#include <renderer/CCTrianglesCommand.h>
#include <renderer/CCRenderer.h>
#include <2d/CCCamera.h>

#include "assets/Textures.h"
#include "debug/Logger.h"
#include "entities/Unit.h"
#include "StateManager.h"

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

namespace map {

    namespace ferr2d {

        using namespace cocos2d;

        //private
        void print_load_error(std::string err_message, std::string file_name) {
            f_assert(sstream_cstr("ferr2d load error occurred: " << err_message << " (" << file_name << ")"));
        }

        bool split_tokens(std::string& data, std::vector<std::string>& tokens, std::string attrib_name, char delimiter = ',') {
            int index = -1;
            int nl_index = -1;
            int co_index = -1;
            if ((index = data.find(attrib_name)) != -1 &&
                (nl_index = data.substr(index).find('\n')) != -1 &&
                (co_index = (data.substr(index).find(':') + 1)) != -1) {
                tokens = split(data.substr(index + co_index, nl_index - co_index), delimiter);
                return true;
            }
            tokens.clear();
            return false;
        }

        //handles load errors when loading terrain, prob should be inline function but gud nuf
#define RETURN_LOAD_ERR(err_message, file_name)                 \
        if (temp != NULL) delete[] temp;                        \
        print_load_error(err_message, file_name);               \
        fclose(f);                                              \
        return NULL;

        //public
        //-- begin terrain class --

        Mat4* base_transform;

        Terrain::Terrain(TerrainData& t_data) {
            terrain_data = &t_data;

            base = Node::create();

            pbody = PhysicsBody::createEdgePolygon(&t_data.collider_points[0], t_data.collider_points.size(), PHYSICSBODY_MATERIAL_DEFAULT, 2.0f);
            pbody->setDynamic(false);
            pbody->setCollisionBitmask(1);
            pbody->setContactTestBitmask(true);
            pbody->setPositionOffset(Vec2(0, -400.0f));

            base->setPhysicsBody(pbody);
            base->setPosition(0.0f, -200.0f);
            base_transform = &base->getNodeToWorldTransform();
            //base->setAnchorPoint(Vec2::ANCHOR_TOP_RIGHT);
            //base->setScale(1.1f, 1.1f);
            root::scene->addChild(base, 1);

            edge_tris.indices = &t_data.indices[t_data.edge_indices_start];
            edge_tris.indexCount = t_data.edge_indices_end;
            edge_tris.verts = &t_data.points[0];
            edge_tris.vertCount = t_data.points.size();

            Texture2D* edge_t = Director::getInstance()->getTextureCache()->addImage("MossyEdges.png");
            edge_t->retain();
            edge_t->setTexParameters({ GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT });

            BlendFunc blend_func;
            blend_func.src = GL_SRC_ALPHA;
            blend_func.dst = GL_ONE_MINUS_SRC_ALPHA;

            edge_tris_cmd.init(0.0f, edge_t->getName(), root::scene->getGLProgramState(), blend_func, edge_tris, *base_transform, Node::FLAGS_RENDER_AS_3D);

            fill_tris.indices = &t_data.indices[t_data.fill_indices_start];
            fill_tris.indexCount = t_data.fill_indices_end;
            fill_tris.verts = &t_data.points[0];
            fill_tris.vertCount = t_data.points.size();

            Texture2D* fill_t = Director::getInstance()->getTextureCache()->addImage("MossyFill.png");
            fill_t->retain();
            fill_t->setTexParameters({ GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT });

            fill_tris_cmd.init(0.0f, fill_t->getName(), root::scene->getGLProgramState(), blend_func, fill_tris, *base_transform, Node::FLAGS_RENDER_AS_3D);

            debug_draw_node = DrawNode::create();
            debug_draw_node->retain();

            debug_draw_on = !debug_draw_on;
            toggle_debug_geometry();
        }
        
        void Terrain::create_debug_geometry(bool show_triangles, bool show_collider_points) {
            base->addChild(debug_draw_node);

            if (show_triangles) {
                for (int n = 1; n < terrain_data->indices.size(); ++n) {
                    if (n % 3 == 0) {
                        debug_draw_node->drawLine(terrain_data->debug_points[terrain_data->indices[n - 1]],
                                                  terrain_data->debug_points[terrain_data->indices[n - 3]],
                                                  Color4F(1.0f, 1.0f, 1.0f, .4f));
                    }else {
                        debug_draw_node->drawLine(terrain_data->debug_points[terrain_data->indices[n - 1]],
                                                  terrain_data->debug_points[terrain_data->indices[n]],
                                                  Color4F(1.0f, 1.0f, 1.0f, .4f));
                    }
                }
            }

            if (show_collider_points) {
                for (int n = 1; n < terrain_data->collider_points.size(); ++n) {
                    debug_draw_node->drawLine(terrain_data->collider_points[n - 1],
                                              terrain_data->collider_points[n],
                                              Color4F(0.0f, 0.0f, 1.0f, .8f));
                }
            }
        }

        void Terrain::remove_debug_geometry() {
            base->removeChild(debug_draw_node);
            debug_draw_node->clear();
        }

        void Terrain::show_debug_geometry(bool show_triangles, bool show_collider_points) {
            create_debug_geometry(show_triangles, show_collider_points);
        }

        void Terrain::hide_debug_geometry() {
            remove_debug_geometry();
        }

        void Terrain::toggle_debug_geometry() {
            if (debug_draw_on = !debug_draw_on) show_debug_geometry();
            else hide_debug_geometry();
        }

        void Terrain::draw() {
            fill_tris_cmd._mv = base->getNodeToWorldTransform();
            edge_tris_cmd._mv = base->getNodeToWorldTransform();
            pbody->setPositionOffset(base->getPosition());
            debug_draw_node->setPosition(base->getPosition());
            Director::getInstance()->getRenderer()->addCommand(&fill_tris_cmd);
            Director::getInstance()->getRenderer()->addCommand(&edge_tris_cmd);
        }

        //-- end terrain class --

        TerrainData* load(std::string file_name) {
            TerrainData* ter = new TerrainData();

            FILE* f = fopen(file_name.c_str(), "r");
            char* temp = NULL;

            if (f != NULL) {
                fseek(f, 0, SEEK_END);
                int file_len = ftell(f);
                rewind(f);

                if (file_len <= 0) { print_load_error("file length cannot be less than 0", file_name); return NULL; }
                temp = new char[file_len];
                fread(temp, 1, file_len, f);

                std::string data = temp;
                std::vector<std::string> tokens;
		        if (split_tokens(data, tokens, "vertex_data:")) {
			        for (int n = 0; n < tokens.size(); n += 2) {
                        V3F_C4B_T2F v;
				        v.vertices.x = std::stof(tokens[n]) * 40.0f;
				        v.vertices.y = std::stof(tokens[n + 1]) * 40.0f;
				        v.colors = Color4B(255, 255, 255, 255);
				        ter->points.push_back(v);

				        Vec2 dv;
				        dv.x = v.vertices.x;
				        dv.y = v.vertices.y;
				        ter->debug_points.push_back(dv);
			        }
		        }else {
                    RETURN_LOAD_ERR("vertex_data attribute missing", file_name);
                }
                if (split_tokens(data, tokens, "indices:")) {
                    for (int n = 0; n < tokens.size(); ++n) {
                        ter->indices.push_back(std::stof(tokens[n]));
                    }
		        }else {
                    RETURN_LOAD_ERR("indices attribute missing", file_name);
                }
                if (split_tokens(data, tokens, "uvs:")) {
                    for (int n = 0; n < tokens.size(); n += 2) {
				        V3F_C4B_T2F& v = ter->points[n / 2];
                        v.texCoords.u = std::stof(tokens[n]);
                        v.texCoords.v = 1 - std::stof(tokens[n + 1]);
                    }
		        }else {
                    RETURN_LOAD_ERR("uvs attribute missing", file_name);
                }
                if (split_tokens(data, tokens, "collider_points:")) {
                    for (int n = 0; n < tokens.size(); n += 2) {
                        Vec2 v;
                        v.x = std::stof(tokens[n]) * 40.0f;
                        v.y = std::stof(tokens[n + 1]) * 40.0f;
				        ter->collider_points.push_back(v);
                    }
		        }else {
                    RETURN_LOAD_ERR("colider_points attribute missing", file_name);
                }
                if (split_tokens(data, tokens, "edge_indices:", '-')) {
                    if (tokens.size() >= 2) {
                        ter->set_edge_index_attrib(std::stof(tokens[0]), std::stof(tokens[1]));
                    }else {
                        RETURN_LOAD_ERR("edge_indices attribute expects more than 2 tokens", file_name);
                    }
		        }else {
                    RETURN_LOAD_ERR("edge_indices attribute missing", file_name);
                }
                if (split_tokens(data, tokens, "fill_indices:", '-')) {
                    if (tokens.size() >= 2) {
				        ter->set_fill_index_attrib(std::stof(tokens[0]), std::stof(tokens[1]));
                    }else {
                        RETURN_LOAD_ERR("fill_indices attribute expects more than 2 tokens", file_name);
                    }
		        }else {
                    RETURN_LOAD_ERR("fill_indices attribute missing", file_name);
                }
		        delete[] temp;
	        }else {
                RETURN_LOAD_ERR("file could not be found", file_name);
            }

            fclose(f);

            return ter;
        }
    };
};
