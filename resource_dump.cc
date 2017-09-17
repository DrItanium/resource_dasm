#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <functional>
#include <phosg/Encoding.hh>
#include <phosg/Filesystem.hh>
#include <phosg/Image.hh>
#include <phosg/Strings.hh>
#include <unordered_map>
#include <vector>

#include "resource_fork.hh"
#include "util.hh"

using namespace std;


void print_usage(const char* name) {
  printf("usage: %s [--copy-handler=FROM,TO | --raw] filename [out_dir]\n",
      name);
}



void write_decoded_image(function<Image(const void*, size_t)> fn,
    const string& out_dir, const string& base_filename, const void* data,
    size_t size, uint32_t type, int16_t id) {

  Image img = fn(data, size);

  uint32_t type_sw = bswap32(type);
  string decoded_filename = string_printf("%s/%s_%.4s_%d.bmp", out_dir.c_str(),
      base_filename.c_str(), (const char*)&type_sw, id);
  img.save(decoded_filename.c_str(), Image::WindowsBitmap);
  printf("... %s\n", decoded_filename.c_str());
}

void write_decoded_image_masked(function<pair<Image, Image>(const void*, size_t)> fn,
    const string& out_dir, const string& base_filename, const void* data,
    size_t size, uint32_t type, int16_t id) {

  pair<Image, Image> imgs = fn(data, size);

  uint32_t type_sw = bswap32(type);

  string decoded_filename = string_printf("%s/%s_%.4s_%d.bmp", out_dir.c_str(),
      base_filename.c_str(), (const char*)&type_sw, id);
  imgs.first.save(decoded_filename.c_str(), Image::WindowsBitmap);
  printf("... %s\n", decoded_filename.c_str());

  decoded_filename = string_printf("%s/%s_%.4s_%d_mask.bmp", out_dir.c_str(),
      base_filename.c_str(), (const char*)&type_sw, id);
  imgs.second.save(decoded_filename.c_str(), Image::WindowsBitmap);
  printf("... %s\n", decoded_filename.c_str());
}

void write_decoded_icnN(const string& out_dir, const string& base_filename,
    const void* data, size_t size, uint32_t type, int16_t id) {
  write_decoded_image_masked(decode_icnN, out_dir, base_filename, data, size,
      type, id);
}

void write_decoded_icsN(const string& out_dir, const string& base_filename,
    const void* data, size_t size, uint32_t type, int16_t id) {
  write_decoded_image_masked(decode_icsN, out_dir, base_filename, data, size,
      type, id);
}

void write_decoded_cicn(const string& out_dir, const string& base_filename,
    const void* data, size_t size, uint32_t type, int16_t id) {
  static auto decode_cicn_local = [](const void* data, size_t size) -> Image {
    return decode_cicn(data, size, -1, -1, -1); // no transparency
  };
  write_decoded_image(decode_cicn_local, out_dir, base_filename, data, size, type, id);
}

void write_decoded_icl8(const string& out_dir, const string& base_filename,
    const void* data, size_t size, uint32_t type, int16_t id) {
  write_decoded_image(decode_icl8, out_dir, base_filename, data, size, type, id);
}

void write_decoded_ics8(const string& out_dir, const string& base_filename,
    const void* data, size_t size, uint32_t type, int16_t id) {
  write_decoded_image(decode_ics8, out_dir, base_filename, data, size, type, id);
}

void write_decoded_icl4(const string& out_dir, const string& base_filename,
    const void* data, size_t size, uint32_t type, int16_t id) {
  write_decoded_image(decode_icl4, out_dir, base_filename, data, size, type, id);
}

void write_decoded_ics4(const string& out_dir, const string& base_filename,
    const void* data, size_t size, uint32_t type, int16_t id) {
  write_decoded_image(decode_ics4, out_dir, base_filename, data, size, type, id);
}

void write_decoded_icon(const string& out_dir, const string& base_filename,
    const void* data, size_t size, uint32_t type, int16_t id) {
  write_decoded_image(decode_icon, out_dir, base_filename, data, size, type, id);
}

void write_decoded_pict(const string& out_dir, const string& base_filename,
    const void* data, size_t size, uint32_t type, int16_t id) {
  write_decoded_image(decode_pict, out_dir, base_filename, data, size, type, id);
}

void write_decoded_snd(const string& out_dir, const string& base_filename,
    const void* data, size_t size, uint32_t type, int16_t id) {

  vector<uint8_t> decoded = decode_snd(data, size);

  uint32_t type_sw = bswap32(type);
  string decoded_filename = string_printf("%s/%s_%.4s_%d.wav", out_dir.c_str(),
      base_filename.c_str(), (const char*)&type_sw, id);
  save_file(decoded_filename, decoded.data(), decoded.size());
  printf("... %s\n", decoded_filename.c_str());
}

void write_decoded_strN(const string& out_dir, const string& base_filename,
    const void* data, size_t size, uint32_t type, int16_t id) {

  vector<string> decoded = decode_strN(data, size);

  uint32_t type_sw = bswap32(type);
  for (size_t x = 0; x < decoded.size(); x++) {
    string decoded_filename = string_printf("%s/%s_%.4s_%d_%lu.txt",
        out_dir.c_str(), base_filename.c_str(), (const char*)&type_sw, id, x);
    save_file(decoded_filename, decoded[x]);
    printf("... %s\n", decoded_filename.c_str());
  }
}



typedef void (*resource_decode_fn)(const string& out_dir,
    const string& base_filename, const void* data, size_t size, uint32_t type,
    int16_t id);

static unordered_map<uint32_t, resource_decode_fn> type_to_decode_fn({
  {RESOURCE_TYPE_CICN, write_decoded_cicn},
  {RESOURCE_TYPE_ICL8, write_decoded_icl8},
  {RESOURCE_TYPE_ICS8, write_decoded_ics8},
  {RESOURCE_TYPE_ICL4, write_decoded_icl4},
  {RESOURCE_TYPE_ICS4, write_decoded_ics4},
  {RESOURCE_TYPE_ICNN, write_decoded_icnN},
  {RESOURCE_TYPE_ICSN, write_decoded_icsN},
  {RESOURCE_TYPE_ICON, write_decoded_icon},
  {RESOURCE_TYPE_PICT, write_decoded_pict},
  {RESOURCE_TYPE_SND , write_decoded_snd},
  //{RESOURCE_TYPE_STRN, write_decoded_strN},
});

static const unordered_map<uint32_t, const char*> type_to_ext({
  {RESOURCE_TYPE_TEXT, "txt"},
  {RESOURCE_TYPE_MOOV, "mov"},
});



enum class SaveRawBehavior {
  Never = 0,
  IfDecodeFails,
  Always,
};

void export_resource(const char* base_filename, const char* resource_filename,
    const char* out_dir, uint32_t type, int16_t id, SaveRawBehavior save_raw) {
  const char* out_ext = "raw";
  if (type_to_ext.count(type)) {
    out_ext = type_to_ext.at(type);
  }

  uint32_t rtype = bswap32(type);
  string out_filename = string_printf("%s/%s_%.4s_%d.%s", out_dir,
      base_filename, (const char*)&rtype, id, out_ext);

  void* data;
  size_t size;
  try {
    load_resource_from_file(resource_filename, type, id, &data, &size);
  } catch (const runtime_error& e) {
    fprintf(stderr, "warning: failed to load resource %08X:%d: %s\n", type, id,
        e.what());
    return;
  }

  bool write_raw = (save_raw == SaveRawBehavior::Always);

  // decode if possible
  resource_decode_fn decode_fn = type_to_decode_fn[type];
  if (decode_fn) {
    try {
      decode_fn(out_dir, base_filename, data, size, type, id);
    } catch (const runtime_error& e) {
      fprintf(stderr, "warning: failed to decode %.4s %d: %s\n",
          (const char*)&rtype, id, e.what());

      // write the raw version if decoding failed and we didn't write it already
      if (save_raw == SaveRawBehavior::IfDecodeFails) {
        write_raw = true;
      }
    }
  } else if (save_raw == SaveRawBehavior::IfDecodeFails) {
    write_raw = true;
  }

  if (write_raw) {
    save_file(out_filename, data, size);
    printf("... %s\n", out_filename.c_str());
  }

  free(data);
}



void disassemble_file(const string& filename, const string& out_dir,
    bool use_data_fork, const unordered_set<uint32_t>& target_types,
    const unordered_set<int16_t>& target_ids, SaveRawBehavior save_raw) {

  // open resource fork if present
  string resource_fork_filename = use_data_fork ? filename :
      first_file_that_exists({
        filename + "/..namedfork/rsrc",
        filename + "/rsrc"});

  // compute the base filename
  size_t last_slash_pos = filename.rfind('/');
  string base_filename = (last_slash_pos == string::npos) ? filename :
      filename.substr(last_slash_pos + 1);

  // get the resources from the file
  vector<pair<uint32_t, int16_t>> resources;
  try {
    resources = enum_file_resources(resource_fork_filename.c_str());
  } catch (const runtime_error& e) {
    fprintf(stderr, "warning: can't enumerate resources; skipping file (%s)\n",
        e.what());
    return;
  }

  for (const auto& it : resources) {
    if (!target_types.empty() && !target_types.count(it.first)) {
      continue;
    }
    if (!target_ids.empty() && !target_ids.count(it.second)) {
      continue;
    }
    export_resource(base_filename.c_str(), resource_fork_filename.c_str(),
        out_dir.c_str(), it.first, it.second, save_raw);
  }
}

void disassemble_path(const string& filename, const string& out_dir,
    bool use_data_fork, const unordered_set<uint32_t>& target_types,
    const unordered_set<int16_t>& target_ids, SaveRawBehavior save_raw) {

  if (isdir(filename)) {
    printf(">>> %s (directory)\n", filename.c_str());

    unordered_set<string> items;
    try {
      items = list_directory(filename);
    } catch (const runtime_error& e) {
      fprintf(stderr, "warning: can\'t list directory: %s\n", e.what());
      return;
    }

    vector<string> sorted_items;
    sorted_items.insert(sorted_items.end(), items.begin(), items.end());
    sort(sorted_items.begin(), sorted_items.end());

    size_t last_slash_pos = filename.rfind('/');
    string base_filename = (last_slash_pos == string::npos) ? filename :
        filename.substr(last_slash_pos + 1);

    string sub_out_dir = out_dir + "/" + base_filename;
    mkdir(sub_out_dir.c_str(), 0777);

    for (const string& item : sorted_items) {
      disassemble_path(filename + "/" + item, sub_out_dir, use_data_fork,
          target_types, target_ids, save_raw);
    }
  } else {
    printf(">>> %s\n", filename.c_str());
    disassemble_file(filename, out_dir, use_data_fork, target_types, target_ids,
        save_raw);
  }
}



int main(int argc, char* argv[]) {

  printf("fuzziqer software macos resource fork disassembler\n\n");

  string filename;
  string out_dir;
  bool use_data_fork = false;
  SaveRawBehavior save_raw = SaveRawBehavior::IfDecodeFails;
  unordered_set<uint32_t> target_types;
  unordered_set<int16_t> target_ids;
  for (int x = 1; x < argc; x++) {
    if (argv[x][0] == '-') {
      if (!strncmp(argv[x], "--copy-handler=", 15)) {
        if (strlen(argv[x]) != 24 || argv[x][19] != ',') {
          printf("incorrect format for --copy-handler: %s\n", argv[x]);
          return 1;
        }
        uint32_t from_type = bswap32(*(uint32_t*)&argv[x][15]);
        uint32_t to_type = bswap32(*(uint32_t*)&argv[x][20]);
        if (!type_to_decode_fn.count(from_type)) {
          printf("no handler exists for type %.4s\n", (const char*)&from_type);
          return 1;
        }
        printf("note: treating %.4s resources as %.4s\n", (const char*)&to_type,
            (const char*)&from_type);
        type_to_decode_fn[to_type] = type_to_decode_fn[from_type];

      } else if (!strncmp(argv[x], "--target-type=", 14)) {
        if (strlen(argv[x]) != 18) {
          printf("incorrect format for --target-type: %s\n", argv[x]);
          return 1;
        }
        uint32_t target_type = bswap32(*(uint32_t*)&argv[x][14]);
        target_types.emplace(target_type);
        printf("note: added %08" PRIX32 " (%.4s) to target types\n",
            target_type, (const char*)&target_type);

      } else if (!strncmp(argv[x], "--target-id=", 12)) {
        int16_t target_id = strtol(&argv[x][12], NULL, 0);
        target_ids.emplace(target_id);
        printf("note: added %04" PRIX16 " (%" PRId16 ") to target types\n",
            target_id, target_id);

      } else if (!strcmp(argv[x], "--skip-decode")) {
        printf("note: skipping all decoding steps\n");
        type_to_decode_fn.clear();

      } else if (!strcmp(argv[x], "--save-raw=no")) {
        printf("note: only writing decoded resources\n");
        save_raw = SaveRawBehavior::Never;

      } else if (!strcmp(argv[x], "--save-raw=if-decode-fails")) {
        printf("note: writing raw resources if decode fails\n");
        save_raw = SaveRawBehavior::IfDecodeFails;

      } else if (!strcmp(argv[x], "--save-raw=yes")) {
        printf("note: writing all raw resources\n");
        save_raw = SaveRawBehavior::Always;

      } else if (!strcmp(argv[x], "--data-fork")) {
        printf("note: reading data forks as resource forks\n");
        use_data_fork = true;

      } else {
        printf("unknown option: %s\n", argv[x]);
        return 1;
      }
    } else {
      if (filename.empty()) {
        filename = argv[x];
      } else if (out_dir.empty()) {
        out_dir = argv[x];
      } else {
        print_usage(argv[0]);
        return 1;
      }
    }
  }

  if (filename.empty()) {
    print_usage(argv[0]);
    return 1;
  }

  if (out_dir.empty()) {
    out_dir = filename + ".out";
  }
  mkdir(out_dir.c_str(), 0777);

  disassemble_path(filename, out_dir, use_data_fork, target_types, target_ids,
      save_raw);

  return 0;
}
