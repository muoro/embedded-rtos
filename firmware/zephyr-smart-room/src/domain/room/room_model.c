#include "room_model.h"

uint32_t room_model_diff_flags(const struct room_state *before, const struct room_state *after)
{
	uint32_t flags = ROOM_CHANGE_NONE;

	if (before->occupied != after->occupied) {
		flags |= ROOM_CHANGE_OCCUPANCY;
	}
	if (before->light_on != after->light_on) {
		flags |= ROOM_CHANGE_LIGHT;
	}
	if (before->contact_open != after->contact_open) {
		flags |= ROOM_CHANGE_CONTACT;
	}
	if (before->alarm != after->alarm) {
		flags |= ROOM_CHANGE_ALARM;
	}

	return flags;
}

static void enforce_light_rules(struct room_model *model)
{
	if (model->state.occupied || model->state.contact_open) {
		model->state.light_on = true;
	}
}

const char *room_alarm_level_to_str(enum alarm_level level)
{
	switch (level) {
	case ALARM_NONE:
		return "none";
	case ALARM_WARNING:
		return "warning";
	case ALARM_ACTIVE:
		return "alarm";
	default:
		return "unknown";
	}
}

void room_model_init(struct room_model *model, uint32_t now_ms)
{
	(void)now_ms;

	*model = (struct room_model){
		.state =
			{
				.occupied = false,
				.light_on = false,
				.contact_open = false,
				.alarm = ALARM_NONE,
			},
	};
}

uint32_t room_model_apply_event(struct room_model *model, enum room_event_type event)
{
	struct room_state before = model->state;

	switch (event) {
	case ROOM_EVENT_OCCUPANCY_TOGGLE:
		model->state.occupied = !model->state.occupied;
		break;
	case ROOM_EVENT_LIGHT_TOGGLE:
		if (model->state.light_on) {
			/* Cancel "turn off" while occupied or contact open. */
			if (!model->state.occupied && !model->state.contact_open) {
				model->state.light_on = false;
			}
		} else {
			model->state.light_on = true;
		}
		break;
	case ROOM_EVENT_CONTACT_TOGGLE:
		model->state.contact_open = !model->state.contact_open;
		break;
	case ROOM_EVENT_ALARM_CYCLE:
		model->state.alarm = (enum alarm_level)((model->state.alarm + 1) % 3);
		break;
	}

	enforce_light_rules(model);
	return room_model_diff_flags(&before, &model->state);
}

bool room_model_set_occupied(struct room_model *model, bool occupied)
{
	struct room_state before = model->state;

	model->state.occupied = occupied;
	enforce_light_rules(model);
	return room_model_diff_flags(&before, &model->state) != ROOM_CHANGE_NONE;
}

bool room_model_set_light_on(struct room_model *model, bool light_on)
{
	struct room_state before = model->state;

	if (!light_on && (model->state.occupied || model->state.contact_open)) {
		model->state.light_on = true;
	} else {
		model->state.light_on = light_on;
	}
	enforce_light_rules(model);
	return room_model_diff_flags(&before, &model->state) != ROOM_CHANGE_NONE;
}

bool room_model_set_contact_open(struct room_model *model, bool contact_open)
{
	struct room_state before = model->state;

	model->state.contact_open = contact_open;
	enforce_light_rules(model);
	return room_model_diff_flags(&before, &model->state) != ROOM_CHANGE_NONE;
}

bool room_model_set_alarm(struct room_model *model, enum alarm_level alarm)
{
	struct room_state before = model->state;

	model->state.alarm = alarm;
	return room_model_diff_flags(&before, &model->state) != ROOM_CHANGE_NONE;
}

const struct room_state *room_model_state(const struct room_model *model)
{
	return &model->state;
}
