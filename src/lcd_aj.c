
static void show_pitch_volume(const char *pitch, uint8_t volume_percent)
{
	char line1[17];
	char line2[17];

	if (volume_percent > 100) {
		volume_percent = 100;
	}

	for (int i = 0; i < 16; i++) line1[i] = ' ';
	line1[16] = '\0';
	line1[0] = 'P'; line1[1] = ':'; line1[2] = ' ';
	if (pitch) {
		for (int i = 0; i < 13 && pitch[i] != '\0'; i++) {
			line1[3 + i] = pitch[i];
		}
	}

	for (int i = 0; i < 16; i++) line2[i] = ' ';
	line2[16] = '\0';
	{
		char buf[16];
		int n = snprintf(buf, sizeof(buf), "V: %u%%", (unsigned)volume_percent);
		if (n < 0) n = 0;
		if (n > 16) n = 16;
		for (int i = 0; i < n && i < 16; i++) {
			line2[i] = buf[i];
		}
	}

	cd_display1(line1);
	cd_display2(line2);
}