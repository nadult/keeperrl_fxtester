#include "particle_system.h"

#include "frect.h"

ParticleSystem::ParticleSystem(FVec2 pos, ParticleSystemDefId def_id, int spawn_time,
							   int num_subsystems)
	: pos(pos), def_id(def_id), spawn_time(spawn_time), subsystems(num_subsystems) {}

void ParticleSystem::kill() {
	subsystems.clear();
	is_dead = true;
}

void defaultAnimateParticle(AnimationContext &ctx, Particle &pinst) {
	const auto &pdef = ctx.pdef;

	float ptime = pinst.particleTime();
	float slowdown = 1.0f / (1.0f + pdef.slowdown.sample(ptime));
	float attract_bottom = pdef.attract_bottom.sample(ptime);

	pinst.pos += pinst.movement * ctx.time_delta;
	pinst.rot += pinst.rot_speed * ctx.time_delta;
	if(slowdown < 1.0f) {
		float factor = pow(slowdown, ctx.time_delta);
		pinst.movement *= factor;
		pinst.rot_speed *= factor;
	}
	if(attract_bottom > 0.0f) {
		float attract_min = 5.0f, attract_max = 10.0f;
		if(pinst.pos.y < attract_min) {
			float dist = attract_min - pinst.pos.y;
			pinst.movement += FVec2(0.0f, dist * attract_bottom);
		}
	}
	pinst.life += ctx.time_delta;
}

float defaultPrepareEmission(AnimationContext &ctx, EmissionState &em) {
	auto &pdef = ctx.pdef;
	auto &edef = ctx.edef;
	float anim_pos = ctx.norm_anim_time;

	em.max_life = pdef.life.sample(anim_pos);

	em.angle = edef.direction.sample(anim_pos);
	em.angle_spread = edef.direction_spread.sample(anim_pos);

	em.strength_min = edef.strength_min.sample(anim_pos);
	em.strength_max = edef.strength_max.sample(anim_pos);

	em.rot_speed_min = edef.rotation_speed_min.sample(anim_pos);
	em.rot_speed_max = edef.rotation_speed_max.sample(anim_pos);

	return edef.frequency.sample(anim_pos) * ctx.time_delta;
}

float AnimationContext::uniformSpread(float spread) { return rand.getDouble(-spread, spread); }
float AnimationContext::uniform(float min, float max) { return rand.getDouble(min, max); }
int AnimationContext::randomSeed() { return rand.getLL() % 1973257861; }

SVec2 AnimationContext::randomTexTile() {
	if(!(pdef.texture_tiles == IVec2(1, 1))) {
		IVec2 tex_tile(rand.get(pdef.texture_tiles.x - 1), rand.get(pdef.texture_tiles.y - 1));
		return SVec2(tex_tile);
	}

	return SVec2(0, 0);
}

void defaultEmitParticle(AnimationContext &ctx, EmissionState &em, Particle &new_inst) {
	new_inst.pos = FVec2();
	float pangle;
	if(em.angle_spread < fconstant::pi)
		pangle = em.angle + ctx.uniformSpread(em.angle_spread);
	else
		pangle = ctx.uniform(0.0f, fconstant::pi * 2.0f);

	FVec2 pdir = angleToVector(pangle);
	float strength = ctx.uniform(em.strength_min, em.strength_max);
	float rot_speed = ctx.uniform(em.rot_speed_min, em.rot_speed_max);
	new_inst.movement = pdir * strength;
	new_inst.rot = ctx.uniform(0.0f, fconstant::pi * 2.0f);
	new_inst.rot_speed = rot_speed * strength;
	new_inst.life = 0.0f;
	new_inst.max_life = em.max_life;
	new_inst.tex_tile = ctx.randomTexTile();
}

array<FVec2, 4> DrawContext::quadCorners(FVec2 pos, FVec2 size, float rotation) const {
	auto corners = FRect(pos - size * 0.5f, pos + size * 0.5f).corners();
	for(auto &corner : corners)
		corner = rotateVector(corner - pos, rotation) + pos;
	return corners;
}

array<FVec2, 4> DrawContext::texQuadCorners(SVec2 tex_tile) const {
	FRect tex_rect(FVec2(1));
	if(!(pdef.texture_tiles == IVec2(1, 1)))
		tex_rect = (tex_rect + FVec2(tex_tile)) * inv_tex_tile;
	return tex_rect.corners();
}

void defaultDrawParticle(DrawContext &ctx, const Particle &pinst, DrawParticle &out) {
	float ptime = pinst.particleTime();
	const auto &pdef = ctx.pdef;

	FVec2 pos = pinst.pos + ctx.ps.pos;
	FVec2 size(pdef.size.sample(ptime));
	float alpha = pdef.alpha.sample(ptime);

	FColor color(pdef.color.sample(ptime) * ctx.ps.params.color[0],
				 alpha); // TODO: by default params dont apply ?
	out.positions = ctx.quadCorners(pos, size, pinst.rot);
	out.tex_coords = ctx.texQuadCorners(pinst.tex_tile);
	out.color = IColor(color);
}
