#!/usr/bin/env python3

import sys, os
import re
import argparse

def add_param(re_par, mod_name, attrs):

  ofi_type = re_par.group(1).lower()
  class_name = re_par.group(2)
  attr_name = re_par.group(4)
  type_name = re_par.group(6)
  method_name = re_par.group(8)

  if not mod_name in attrs:
    attrs[mod_name] = {}

  if not class_name in attrs[mod_name]:
    attrs[mod_name][class_name] = []

  if "::" in type_name:
    parts = type_name.split("::", maxsplit = 1)
    type_name = parts[1]
    type_mod = parts[0]
  else:
    type_mod = None

  attrs[mod_name][class_name].append({
    "Attribute name": attr_name,
    "Attribute module": mod_name,
    "Type name": type_name,
    "Type module": type_mod,
    "Method name": method_name,
    "OFI type": ofi_type
  })

def add_type_base(re_par, mod_name, type_base):

  type_name = re_par.group(1)
  base_name = re_par.group(3)

  type_base[type_name] = base_name

def extract_tags(path, filename, attrs, type_base, defines):

  re_ifdef = re.compile(r"^\s*#\s*ifdef\s*([a-zA-Z0-9_]+)\s*")
  re_ifndef = re.compile(r"^\s*#\s*ifndef\s*([a-zA-Z0-9_]+)\s*")
  re_anyif = re.compile(r"^\s*#\s*if\s*")
  re_else = re.compile(r"^\s*#\s*else")
  re_endif = re.compile(r"^\s*#\s*endif")

  pathsep = "\\\\" if os.sep == "\\" else os.sep
  m = re.search(pathsep + '(gm[A-Z][a-zA-Z]+)' + pathsep, os.path.normpath(path))
  mod_name = m.group(1) if m is not None else "Unknown"

  defines_stack = []

  with open("%s/%s" % (path, filename), "r") as fin:
    for line in fin:

      if defines is not None:

        m_ifdef = re_ifdef.search(line)
        m_ifndef = re_ifndef.search(line)
        m_anyif = re_anyif.search(line)
        m_else = re_else.search(line)
        m_endif = re_endif.search(line)

        if m_ifdef is not None:
          if m_ifdef.group(1) not in defines:
            defines_stack.append(False)
          else:
            defines_stack.append(True if len(defines_stack) == 0 else defines_stack[-1])

        elif m_ifndef is not None:
          if m_ifndef.group(1) in defines:
            defines_stack.append(False)
          else:
            defines_stack.append(True if len(defines_stack) == 0 else defines_stack[-1])

        elif m_anyif is not None:
          defines_stack.append(True if len(defines_stack) == 0 else defines_stack[-1])

        elif m_else is not None:
          defines_stack[-1] = not defines_stack[-1]

        elif m_endif is not None:
          defines_stack.pop()

        if len(defines_stack) > 0 and defines_stack[-1] == False:
          continue

      re_par = re.search(r'^GM_OFI_(PARAM|POINTER)2?\(\s*([a-zA-Z0-9]+)([^a-zA-Z0-9]*)([a-zA-Z0-9]+)([^a-zA-Z0-9]*)([a-zA-Z0-9:]+)([^a-zA-Z0-9]*)([a-zA-Z0-9:]+)\)', line)
      if re_par is not None:
        add_param(re_par, mod_name, attrs)
        continue

      re_par = re.search(r'^GM_OFI_DEFINE_SUB\(\s*([a-zA-Z0-9]+)([^a-zA-Z0-9]*)([a-zA-Z0-9]+)\)', line)
      if re_par is not None:
        add_type_base(re_par, mod_name, type_base)


def main(argv):

  suffices = [ ".cpp", ".hh" ]

  parser = argparse.ArgumentParser(description='Generate Doxygen documentation over XML attributes in gramods')
  parser.add_argument('--modules', metavar='path', required=True,
                      help='The path to the modules folder.')
  parser.add_argument('--output', metavar='path', required=True,
                      help='The path to the folder to put the generated files in.')
  parser.add_argument('--defines', metavar='def', default=None,
                      help='A comma separated list of defines.')

  args = parser.parse_args()

  defines = args.defines.split(',') if args.defines is not None else None

  attrs = {}
  type_base = {}
  for dirName, subdirList, fileList in os.walk(args.modules):
    for filename in fileList:
      for suffix in suffices:
        if filename.rfind(suffix) - len(filename) + len(suffix) == 0:
          extract_tags(dirName, filename, attrs, type_base, defines)

  # attrs[(mod, class)][N][0-2] (attr, type, method)
  for mod in attrs:

    print (f"Creating documentation for {mod}")
    count = 0

    mod_string = f"/**\\page xml-{mod} XML Documentation for {mod}\n\n"
    mod_string += f"""This page lists elements defined in the {mod} module and parameters (P), containers (C), their types and their corresponding C++ methods. Parameters are set either as attributes or with the param tag, e.g. &lt;param name="fullscreen" value="true"&gt;. Containers are typically automatically associated with the child type, through the \\link gramods::gmCore::Object::getDefaultKey() Object::getDefaultKey() \\endlink method, however it can be explicitly controlled through the \\c KEY="container" attribute. Observe also that futher documentation for how an attribute should be used is sometimes available adjacent to the C++ methods.\n\n"""
    mod_string += """XML attributes are parsed by stream operators. For more information about how this is done, refer to the documentation of the respective stream operator in the \\ref gramods namespace.\n\n"""

    for clas in sorted(attrs[mod].keys()):

      if clas == "CLASS":
        continue

      #mod_string += f"\n\n\\link gramods::{mod}::{clas} \\b {clas} \\endlink\n\n"
      if clas not in type_base:
        mod_string += f"\n\n\\section xml-{mod}-{clas} {clas}\n\n"
      else:
        mod_string += f"\n\n\\section xml-{mod}-{clas} {clas} (extends {type_base[clas]})\n\n"
      mod_string += f"\n\n\\copybrief gramods::{mod}::{clas} \\link gramods::{mod}::{clas} (more)\\endlink\n\n"
      attrs[mod][clas].sort(key=lambda e: e["Attribute name"])

      mod_string += f"<table><tr> <th> </th> <th>Attribute</th> <th>Type</th> <th>Method</th> <th>Documentation</th> </tr>\n"

      for data in attrs[mod][clas]:

        attr_name = data["Attribute name"]
        mod_name = data["Attribute module"]
        type_name = data["Type name"]
        type_mod = data["Type module"]
        method_name = data["Method name"]
        ofi_type = data["OFI type"]

        cleanmethod = method_name.replace(clas, '')

        if type_mod is None and (ofi_type == 'pointer' or ofi_type == 'pointer2'):
          fulltype = "gramods::" + mod + "::" + type_name
        elif type_mod is None:
          fulltype = type_name
        elif type_mod[:2] == "gm":
          fulltype = "gramods::" + type_mod + "::" + type_name
        else:
          fulltype = type_mod + "::" + type_name

        if ofi_type == 'param' or ofi_type == 'param2':
          mod_string += f"""
<tr>
  <td>P</td>
  <td>{attr_name}</td>
  <td>{type_name}</td>
  <td>\\link gramods::{mod}::{clas}::{method_name}({fulltype}) {cleanmethod} \\endlink </td>
  <td> \\copybrief gramods::{mod}::{clas}::{method_name}({fulltype}) </td>
</tr>
"""
        elif ofi_type == 'pointer' or ofi_type == 'pointer2':
          mod_string += f"""
<tr>
  <td>C</td>
  <td>{attr_name}</td>
  <td>\\link {fulltype} {type_name} \\endlink</td>
  <td>\\link gramods::{mod}::{clas}::{method_name}(std::shared_ptr<{type_name}>) {cleanmethod} \\endlink </td>
  <td> \\copybrief gramods::{mod}::{clas}::{method_name}(std::shared_ptr<{type_name}>) </td>
</tr>
"""
        else:
          mod_string += f"""
<tr>
  <td>?</td>
  <td>{attr_name}</td>
  <td>{type_name} \\endlink</td>
  <td>\\link gramods::{mod}::{clas}::{method_name}(std::shared_ptr<{type_name}>) {cleanmethod} \\endlink </td>
  <td> \\copybrief gramods::{mod}::{clas}::{method_name}(std::shared_ptr<{type_name}>)</td>
</tr>
"""

        count += 1
      mod_string += f"</table>\n"

    mod_string += "\n*/\n"

    with open(args.output + "/" + mod + ".hh", "w") as fout:
      fout.write(mod_string)
    print (f"Documented {count} attributes")

if __name__ == "__main__":
  main(sys.argv)

