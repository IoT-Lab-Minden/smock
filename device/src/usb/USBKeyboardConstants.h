/**
 * \file USBKeyboardConstants.h
 * \brief Contains the constants and structs needed for the Keyboard Device
 *
 * Contains needed structs for the keyboard device for holding current keyboard state and
 * for working with the different usage codes for the key.
 */

#ifndef USBKEYBOARDCONSTANTS_H_
#define USBKEYBOARDCONSTANTS_H_
#include "usblib/usbhid.h"

namespace usbdevice {

	/**
	 * \var const uint8_t UI_CAPS_LOCK
	 *
	 * Usage code of caps lock.
	 */
	const uint8_t UI_CAPS_LOCK = 0x00000001;

	/**
	 * \var const uint8_t UI_SCROLL_LOCK
	 *
	 * Usage code of scroll lock.
	 */
	const uint8_t UI_SCROLL_LOCK = 0x00000002;

	/**
	 * \var const uint8_t UI_NUM_LOCK
	 *
	 * Usage code of num lock.
	 */
	const uint8_t UI_NUM_LOCK = 0x00000003;

	/**
	 * \var const char TAB
	 *
	 * Usage code of tab.
	 */
	const char TAB = 0x2b;

	/**
	 * const char KEY_TAB
	 *
	 * Self defined code for tab.
	 */
	const char KEY_TAB = 202;

	/**
	 * \var const uint8_t DEL
	 *
	 * Code for the delete key
	 */
	const uint8_t DEL = 0x4c;

	/**
	 * \var const uint8_t ENTER
	 *
	 * Code for the enter key
	 */
	const char ENTER = UNICODE_RETURN;

	/**
	 * \var g_sKeyboardState
	 * \brief holds USB keyboard state.
	 *
	 * Holds a pending special key press for the Caps Lock, Scroll Lock, or Num Lock keys.
	 */
	struct {
		uint8_t ui8Special;
		uint8_t pending;
	}g_sKeyboardState;

	/**
	 * \struct sUsageEntry
	 * \brief The look up table entries for usage codes.
	 *
	 * Containg two char. The first is the char value and the second one is the matching
	 * value for the HID key.
	 */
	typedef struct {
		char cChar;
		char cUsage;
	} sUsageEntry;

	/**
	 * \var const sUsageEntry g_pcUsageCodes[]
	 *
	 * The un-shifted HID usage codes used by the graphical keyboard.
	 */
	const sUsageEntry g_pcUsageCodes[] = { {'q', HID_KEYB_USAGE_Q}, {
			'w', HID_KEYB_USAGE_W},
		{	'e', HID_KEYB_USAGE_E}, {
			'r', HID_KEYB_USAGE_R},
		{	't', HID_KEYB_USAGE_T}, {
			'z', HID_KEYB_USAGE_Y},
		{	'u', HID_KEYB_USAGE_U}, {
			'i', HID_KEYB_USAGE_I},
		{	'o', HID_KEYB_USAGE_O}, {
			'p', HID_KEYB_USAGE_P},
		{	'a', HID_KEYB_USAGE_A}, {
			's', HID_KEYB_USAGE_S},
		{	'd', HID_KEYB_USAGE_D}, {
			'f', HID_KEYB_USAGE_F},
		{	'g', HID_KEYB_USAGE_G}, {
			'h', HID_KEYB_USAGE_H},
		{	'j', HID_KEYB_USAGE_J}, {
			'k', HID_KEYB_USAGE_K},
		{	'l', HID_KEYB_USAGE_L}, {
			'y', HID_KEYB_USAGE_Z},
		{	'x', HID_KEYB_USAGE_X}, {
			'c', HID_KEYB_USAGE_C},
		{	'v', HID_KEYB_USAGE_V}, {
			'b', HID_KEYB_USAGE_B},
		{	'n', HID_KEYB_USAGE_N}, {
			'm', HID_KEYB_USAGE_M},
		{	'0', HID_KEYB_USAGE_0}, {
			'1', HID_KEYB_USAGE_1},
		{	'2', HID_KEYB_USAGE_2}, {
			'3', HID_KEYB_USAGE_3},
		{	'4', HID_KEYB_USAGE_4}, {
			'5', HID_KEYB_USAGE_5},
		{	'6', HID_KEYB_USAGE_6}, {
			'7', HID_KEYB_USAGE_7},
		{	'8', HID_KEYB_USAGE_8}, {
			'9', HID_KEYB_USAGE_9},
		{	'-', HID_KEYB_USAGE_MINUS},
		{	'=', HID_KEYB_USAGE_EQUAL}, {
			'\'',
			HID_KEYB_USAGE_FQUOTE},
		{	'[', HID_KEYB_USAGE_LBRACKET},
		{	']', HID_KEYB_USAGE_RBRACKET},
		{	';', HID_KEYB_USAGE_SEMICOLON},
		{	' ', HID_KEYB_USAGE_SPACE},
		{	'/', HID_KEYB_USAGE_FSLASH}, {
			'\\',
			HID_KEYB_USAGE_BSLASH},
		{	'.', HID_KEYB_USAGE_PERIOD},
		{	',', HID_KEYB_USAGE_COMMA}, {
			UI_CAPS_LOCK,
			HID_KEYB_USAGE_CAPSLOCK},
		{	UI_SCROLL_LOCK,
			HID_KEYB_USAGE_SCROLLOCK},
		{	UI_NUM_LOCK,
			HID_KEYB_USAGE_NUMLOCK},
		{	UNICODE_BACKSPACE,
			HID_KEYB_USAGE_BACKSPACE},
		{	UNICODE_RETURN,
			HID_KEYB_USAGE_ENTER},
		{	(char)202,
			TAB},};

	/**
	 * \var const uint32_t ui32NumUsageCodes
	 *
	 * Number of entries in the usagecode array
	 */
	const uint32_t ui32NumUsageCodes = sizeof(g_pcUsageCodes) / sizeof(sUsageEntry);

	/**
	 * \var const sUsageEntry g_pcUsageCodesShift[]
	 *
	 * The shifted HID usage codes that are used by the graphical keyboard.
	 */
	const sUsageEntry g_pcUsageCodesShift[] = {
		{	')', HID_KEYB_USAGE_0}, {'!', HID_KEYB_USAGE_1},
		{	'@', HID_KEYB_USAGE_2}, {'#', HID_KEYB_USAGE_3},
		{	'$', HID_KEYB_USAGE_4}, {'%', HID_KEYB_USAGE_5},
		{	'^', HID_KEYB_USAGE_6}, {'&', HID_KEYB_USAGE_7},
		{	'*', HID_KEYB_USAGE_8}, {'(', HID_KEYB_USAGE_9}, {
			'?', HID_KEYB_USAGE_FSLASH},
		{	'+', HID_KEYB_USAGE_EQUAL}, {':', HID_KEYB_USAGE_SEMICOLON}, {
			'_', HID_KEYB_USAGE_MINUS},
		{	'~', HID_KEYB_USAGE_BQUOTE}, {'|', HID_KEYB_USAGE_BSLASH}, {
			'\"', HID_KEYB_USAGE_FQUOTE},};

	/**
	 * \var const uint32_t ui32NumUsageCodesShift
	 *
	 * Number of entries in the usagecode array
	 */
	const uint32_t ui32NumUsageCodesShift = sizeof(g_pcUsageCodes) / sizeof(sUsageEntry);

}



#endif /* USBKEYBOARDCONSTANTS_H_ */
