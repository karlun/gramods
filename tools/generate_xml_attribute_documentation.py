#!/usr/bin/env python3

import sys, os
import re
import argparse

def extract_tags(path, filename, attrs):

  pathsep = "\\\\" if os.sep == "\\" else os.sep
  m = re.search(pathsep + '(gm[A-Z][a-zA-Z]+)' + pathsep, os.path.normpath(path))
  mod_name = m.group(1) if m is not None else "Unknown"

  with open("%s/%s" % (path, filename), "r") as fin:

    for line in fin:

      re_par = re.search('^GM_OFI_(PARAM|POINTER)\(([a-zA-Z0-9]+)([^a-zA-Z0-9]*)([a-zA-Z0-9]+)([^a-zA-Z0-9]*)([a-zA-Z0-9:]+)([^a-zA-Z0-9]*)([a-zA-Z0-9:]+)\)', line)
      if re_par is None:
        continue

      ofi_type = re_par.group(1).lower()
      class_name = re_par.group(2)
      attr_name = re_par.group(4)
      type_name = re_par.group(6)
      method_name = re_par.group(8)

      if not mod_name in attrs:
        attrs[mod_name] = {}

      if not class_name in attrs[mod_name]:
        attrs[mod_name][class_name] = []

      attrs[mod_name][class_name].append([attr_name, type_name, method_name, ofi_type])

def first_element(e):
  return e[0]

def main(argv):

  suffices = [ ".cpp", ".hh" ]

  parser = argparse.ArgumentParser(description='Generate Doxygen documentation over XML attributes in gramods')
  parser.add_argument('--modules', metavar='path', required=True,
                      help='The path to the modules folder.')
  parser.add_argument('--output', metavar='path', required=True,
                      help='The path to the folder to put the generated files in.')

  args = parser.parse_args()

  attrs = {}
  for dirName, subdirList, fileList in os.walk(args.modules):
    for filename in fileList:
      for suffix in suffices:
        if filename.rfind(suffix) - len(filename) + len(suffix) == 0:
          extract_tags(dirName, filename, attrs)

  # attrs[(mod, class)][N][0-2] (attr, type, method)
  for mod in attrs:

    print (f"Creating documentation for {mod}")
    count = 0

    mod_string = f"/**\page xml-{mod.lower()} XML Documentation for {mod}\n\n"
    mod_string += f"""This page lists elements defined in the {mod} module and parameters (P), containers (C), their types and their corresponding C++ methods. Parameters are set either as attributes or with the param tag, e.g. &lt;param name="fullscreen" value="true"&gt;. Containers are typically automatically associated with the child type, through the \\link gramods::gmCore::Object::getDefaultKey() Object::getDefaultKey() \\endlink method, however it can be explicitly controlled through the \\c AS="container" attribute. Observe also that futher documentation for how an attribute should be used is sometimes available adjacent to the C++ methods.\n\n"""
    mod_string += """XML attributes are parsed by stream operators. For more information about how this is done, refer to the documentation of the respective stream operator in the \\ref gramods namespace.\n\n"""

    for clas in sorted(attrs[mod].keys()):

      if clas == "CLASS":
        continue

      #mod_string += f"\n\n\\link gramods::{mod}::{clas} \\b {clas} \\endlink\n\n"
      mod_string += f"\n\n\\section xml-{mod}-{clas} {clas}\n\n"
      mod_string += f"\n\n\\copybrief gramods::{mod}::{clas} \\link gramods::{mod}::{clas} (more)\\endlink\n\n"
      attrs[mod][clas].sort(key=first_element)

      mod_string += f"<table><tr> <th> </th> <th>Attribute</th> <th>Type</th> <th>Method</th> <th>Documentation</th> </tr>\n"

      for data in attrs[mod][clas]:

        theattr = data[0]
        thetype = data[1]
        themethod = data[2]
        ofi_type = data[3]
        cleanmethod = themethod.replace(clas, '')
        fulltype = thetype if "::" in thetype else mod + "::" + thetype

        #mod_string += f"\li \b {theattr} = {thetype} (\link gramods::{mod}::{themethod}() {themethod} \endlink) <br/> \copybrief gramods::{mod}::{themethod}()\n"
        if ofi_type == 'param':
          if "::" in thetype and thetype[:2] == "gm":
            mod_string += f"<tr><td>P</td><td>{theattr}</td><td>\\link gramods::{thetype} {thetype} \\endlink </td><td>\\link gramods::{mod}::{themethod}() {cleanmethod} \\endlink </td><td> \\copybrief gramods::{mod}::{themethod}() </td></tr>\n"
          else:
            mod_string += f"<tr><td>P</td><td>{theattr}</td><td>{thetype}</td><td>\\link gramods::{mod}::{themethod}() {cleanmethod} \\endlink </td><td> \\copybrief gramods::{mod}::{themethod}() </td></tr>\n"
        elif ofi_type == 'pointer':
          mod_string += f"<tr><td>C</td><td>{theattr}</td><td>\\link gramods::{fulltype} {thetype} \\endlink</td><td>\\link gramods::{mod}::{themethod}() {cleanmethod} \\endlink </td><td> \\copybrief gramods::{mod}::{themethod}() </td></tr>\n"
        else:
          mod_string += f"<tr><td>?</td><td>{theattr}</td><td>{thetype} \\endlink</td><td>\\link gramods::{mod}::{themethod}() {cleanmethod} \\endlink </td><td> \\copybrief gramods::{mod}::{themethod}() </td></tr>\n"

        count += 1
      mod_string += f"</table>\n"

    mod_string += "\n*/\n"

    with open(args.output + "/" + mod + ".hh", "w") as fout:
      fout.write(mod_string)
    print (f"Documented {count} attributes")

if __name__ == "__main__":
  main(sys.argv)

