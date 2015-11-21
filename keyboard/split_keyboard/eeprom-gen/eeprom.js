var EECONFIG_MAGIC_NUMBER                     = [0xED, 0xFE];
var EECONFIG_MAGIC                            = 0;
var EECONFIG_DEBUG                            = 2;
var EECONFIG_DEFAULT_LAYER                    = 3;
var EECONFIG_KEYMAP                           = 4;
var EECONFIG_MOUSEKEY_ACCEL                   = 5;
var EECONFIG_BACKLIGHT                        = 6;
/*debug bit */
var EECONFIG_DEBUG_ENABLE                     = (1<<0);
var EECONFIG_DEBUG_MATRIX                     = (1<<1);
var EECONFIG_DEBUG_KEYBOARD                   = (1<<2);
var EECONFIG_DEBUG_MOUSE                      = (1<<3);
/*keyconf bit */
var EECONFIG_KEYMAP_SWAP_CONTROL_CAPSLOCK     = (1<<0);
var EECONFIG_KEYMAP_CAPSLOCK_TO_CONTROL       = (1<<1);
var EECONFIG_KEYMAP_SWAP_LALT_LGUI            = (1<<2);
var EECONFIG_KEYMAP_SWAP_RALT_RGUI            = (1<<3);
var EECONFIG_KEYMAP_NO_GUI                    = (1<<4);
var EECONFIG_KEYMAP_SWAP_GRAVE_ESC            = (1<<5);
var EECONFIG_KEYMAP_SWAP_BACKSLASH_BACKSPACE  = (1<<6);
var EECONFIG_KEYMAP_NKRO                      = (1<<7);

var EECONFIG_AES_KEY        = 0x0010; // 16 bytes

var EECONFIG_RF_POWER       = 0x0020; // 1 byte
var EECONFIG_RF_CHANNEL     = 0x0021; // 1 byte
var EECONFIG_RF_DATA_RATE   = 0x0022; // 1 byte
var EECONFIG_RF_ADDRESS_LEN = 0x0023; // 1 byte
var EECONFIG_MESSAGE_ID0    = 0x0028; // 4  bytes
var EECONFIG_MESSAGE_ID1    = 0x002C; // 4  bytes

var EECONFIG_DEVICE_ADDR_0 =   0x0030;// 5  bytes
var EECONFIG_DEVICE_ADDR_1 =   0x0035;// 5  bytes

function toHexString(thing) {
  var result = "";
  for (var i = 0, len = thing.length; i < len; i++) {
    result += thing[i].toString(16);
  }
  return result;
}

function genRandomBytes(len) {
  var buf = new Uint8Array(len);
  window.crypto.getRandomValues(buf);
  return buf;
}

function copyRegion(dest, src, addr) {
  for (var i = 0, len = src.length; i < len; i++) {
    dest[i+addr] = src[i];
  }
}

function getField8(id, mask) {
  if (mask === undefined) {
    mask = 0xff;
  }
  var elem = document.getElementById(id);
  if ( elem.value === "" ) {
    elem.value = 0;
  }
  if ( elem.value > mask ) {
    elem.value = mask;
  }
  return ( elem.value & mask);
}

function getBits(id) {
  var res = 0;
  for (var i = 0; i < 8; i++) {
    var elem = document.getElementById(id + i);
    if (elem) {
      res |= elem.checked << i;
    }
  }
  return res;
}

function getRadio(name) {
  var radioButtons = document.getElementsByName(name);
  for (var i = 0, len = radioButtons.length; i < len; i++) {
    if (radioButtons[i].checked) {
      return radioButtons[i].value;
    }
  }
  return res;
}

var data = {
  memory : [],
  key    : genRandomBytes(16),
  rf_channel : genRandomBytes(1)[0] & 0x7f,
  rf_power : 3,
  rf_data_rate : 2,
  rf_address_len : 3,
  addr0  : genRandomBytes(5),
  addr1  : genRandomBytes(5),
  keyConf : 0,
  debug : 0,
  layer : 0,
};

for (var i = 0; i < 64; i++) {
  data.memory.push(0);
}

function generateHex() {
  // tmk
  copyRegion(data.memory , EECONFIG_MAGIC_NUMBER , EECONFIG_MAGIC);
  copyRegion(data.memory , [data.layer    ]      , EECONFIG_DEFAULT_LAYER);
  copyRegion(data.memory , [data.keyConf  ]      , EECONFIG_KEYMAP);
  copyRegion(data.memory , [data.backlight]      , EECONFIG_BACKLIGHT);
  copyRegion(data.memory , [data.debug    ]      , EECONFIG_DEBUG);
  copyRegion(data.memory , [data.mouseaccel]     , EECONFIG_MOUSEKEY_ACCEL);

  // wireless
  copyRegion(data.memory, data.key,        EECONFIG_AES_KEY);
  copyRegion(data.memory, data.addr0,      EECONFIG_DEVICE_ADDR_0);
  copyRegion(data.memory, data.addr1,      EECONFIG_DEVICE_ADDR_1);
  data.memory[EECONFIG_RF_CHANNEL] = data.rf_channel;
  data.memory[EECONFIG_RF_DATA_RATE] = data.rf_data_rate;
  data.memory[EECONFIG_RF_ADDRESS_LEN] = data.rf_address_len;
  data.memory[EECONFIG_RF_POWER] = data.rf_power;

  var temp = new JVIntelHEX(data.memory.slice(), 16, 0x0000, true); temp.createRecords();
  data.generatedFile = temp.getHEXFile();

  var rf_settings = ""
      rf_settings += ("aes_key: " + toHexString(data.key));
      rf_settings += ("<br/>");
      rf_settings += ("rf_power: " + data.rf_power);
      rf_settings += (" (" + (-18 + 6*data.rf_power) + " dbm)");
      rf_settings += ("<br/>");
      rf_settings += ("rf_channel: " + data.rf_channel);
      rf_settings += (" (" + (2400 + data.rf_channel)/1000 + " GHz)");
      rf_settings += ("<br/>");
      rf_settings += ("mac addr0:  " + toHexString(data.addr0));
      rf_settings += ("<br/>");
      rf_settings += ("mac addr1: " + toHexString(data.addr1));
      rf_settings += ("<br/>");

  document.getElementById('rf').innerHTML = rf_settings;
  document.getElementById('output').value = data.generatedFile;
}

function updateSettings() {
  data.backlight = getField8("backlight");
  data.mouseaccel = getField8("mouseaccel");
  data.rf_power = getField8("rf_power", 0x03);
  data.rf_data_rate = parseInt(getRadio("rf_data_rate"));
  data.rf_address_len = parseInt(getRadio("rf_address_len"));
  data.debug = getBits("d");
  data.layer = getBits("l");
  data.keyConf = getBits("k");

  generateHex();
}

