import uiScriptLocale

ROOT = "d:/ymir work/ui/public/"

window = {
	"name": "WhisperDialog",
	"style": ("movable", "float",),

	"x": 0,
	"y": 0,

	"width": 450,
	"height": 200,

	"children":
	(
		{
			"name": "board",
			"type": "thinboard",
			"style": ("attach",),

			"x": 0,
			"y": 0,

			"width": 450,
			"height": 200,

			"children":
			(
				{
					"name": "name_slot",
					"type": "image",
					"style": ("attach",),

					"x": 10,
					"y": 10,

					"image": "d:/ymir work/ui/public/Parameter_Slot_06.sub",

					"children":
					(
						{
							"name": "titlename",
							"type": "text",

							"x": 3,
							"y": 3,

							"text": uiScriptLocale.WHISPER_NAME,
						},
						{
							"name": "titlename_edit",
							"type": "editline",

							"x": 3,
							"y": 3,

							"width": 240,
							"height": 17,

							"input_limit": PLAYER_NAME_MAX_LEN,

							"text": uiScriptLocale.WHISPER_NAME,
						},
					),
				},

				{
					"name": "gamemastermark",
					"type": "expanded_image",
					"style": ("attach",),

					"x": 306,
					"y": 6,

					"x_scale": 0.2,
					"y_scale": 0.2,

					"image": LOCALE_PATH + "/effect/ymirred.tga",
				},
				{
					"name": "ignorebutton",
					"type": "toggle_button",

					"x": 245,
					"y": 10,

					"text": uiScriptLocale.WHISPER_BAN,

					"default_image": "d:/ymir work/ui/public/small_thin_button_01.sub",
					"over_image": "d:/ymir work/ui/public/small_thin_button_02.sub",
					"down_image": "d:/ymir work/ui/public/small_thin_button_03.sub",
				},
				{
					"name": "reportviolentwhisperbutton",
					"type": "button",

					"x": 245,
					"y": 10,

					"text": uiScriptLocale.WHISPER_REPORT,

					"default_image": "d:/ymir work/ui/public/large_button_01.sub",
					"over_image": "d:/ymir work/ui/public/large_button_02.sub",
					"down_image": "d:/ymir work/ui/public/large_button_03.sub",
				},
				{
					"name": "acceptbutton",
					"type": "button",

					"x": 235,
					"y": 9,

					"text": uiScriptLocale.OK,

					"default_image": "d:/ymir work/ui/public/small_thin_button_01.sub",
					"over_image": "d:/ymir work/ui/public/small_thin_button_02.sub",
					"down_image": "d:/ymir work/ui/public/small_thin_button_03.sub",
				},
				{
					"name": "minimizebutton",
					"type": "button",

					"x": 380,
					"y": 12,

					"tooltip_text": uiScriptLocale.MINIMIZE,

					"default_image": "d:/ymir work/ui/public/minimize_button_01.sub",
					"over_image": "d:/ymir work/ui/public/minimize_button_02.sub",
					"down_image": "d:/ymir work/ui/public/minimize_button_03.sub",
				},
				{
					"name": "closebutton",
					"type": "button",

					"x": 406,
					"y": 12,

					"tooltip_text": uiScriptLocale.CLOSE,

					"default_image": "d:/ymir work/ui/public/close_button_01.sub",
					"over_image": "d:/ymir work/ui/public/close_button_02.sub",
					"down_image": "d:/ymir work/ui/public/close_button_03.sub",
				},
				{
					"name": "scrollbar",
					"type": "thin_scrollbar",

					"x": 406,
					"y": 35,

					"size": 120,
				},
				{
					"name": "editbar",
					"type": "bar",

					"x": 10,
					"y": 140,

					"width": 420,
					"height": 50,

					"color": 0x77000000,

					"children":
					(
						{
							"name": "chatline",
							"type": "editline",

							"x": 5,
							"y": 5,

							"width": 350,
							"height": 40,

							"with_codepage": 1,
							"input_limit": 40,
							"limit_width": 350,
							"multi_line": 1,
						},
						{
							"name": "sendbutton",
							"type": "button",

							"x": 360,
							"y": 10,

							"text": uiScriptLocale.WHISPER_SEND,

							"default_image": "d:/ymir work/ui/public/xlarge_thin_button_01.sub",
							"over_image": "d:/ymir work/ui/public/xlarge_thin_button_02.sub",
							"down_image": "d:/ymir work/ui/public/xlarge_thin_button_03.sub",
						},
					),
				},
			),
		},
	),
}
