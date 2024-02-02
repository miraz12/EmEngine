import os

#include "../Engine/include/engine_api.hpp"

// Declare the C function
fn C.ECS_loadScene(file &char)

@[export: 'game_initialize']
fn game_initialize() {
	println('[game] initialize')
	scene_file := os.resource_abs_path('../../resources/scene.yaml')
	C.ECS_loadScene(scene_file.str)
}

@[export: 'game_update']
fn game_update(dt f32) {
	println('[game] update')
}
