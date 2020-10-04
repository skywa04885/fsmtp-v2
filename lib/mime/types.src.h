/*
	Copyright [2020] [Luke A.C.A. Rieff]

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#ifndef _LIB_MIME_TYPES_H
#define _LIB_MIME_TYPES_H

#include "../default.h"

namespace FSMTP::MIME {
  enum FileTypes {
    FileTypesStart,
    AACAudio, // .aac
    AbiWord, // .abw
    FreeArchive, // .arc
    AviAudio, // .avi
    KindleEbook, // .azw
    Binary, // .bin
    BitmapGraphics, // bmp
    BzipArchive, // .bz
    Bzip2Archive, // .bz2
    CShellScript, // .csh
    CascadingStyleSheets, // .css
    CommaSeparatedValues, // csv
    MicrosoftWord, // .doc
    MicrosoftWordOpenXML, // docx
    MSOpenTypeFonts, // .eot
    ElectronicPublication, // .epub
    GZIPArchive, // .gz
    AnimatedGIF, // .gif
    HypertextMarkupLanguage, // .html - .htm
    IconFormat, // .ico
    IcalenderFormat, // .ics
    JavaArchive, // .jar
    JPEGImage, // .jpeg - .jpg
    JavaScript, // .js
    JSON, // .json
    JSON_LD, // .jsonld
    MusicalInstrumentDigitalInterface, // .midi - .mid
    JavaScriptModule, // .mjs
    MP3Audio, // .mp3
    MPEGVideo, // .mpeg
    AppleInstallerPackage, // .mpkg
    OpenDocumentPresentation, // .odp
    OpenDocumentSpreadSheet, // .ods
    OpenDocumentText, // .odt
    OGGAudio, // .ogg
    OGGVideo, // .ogv
    OpusAudio, // .opus
    OpenTypeFont, // .otf
    PortableNetworkGraphics, // .png
    AdobePortableDocumentFormat, // .pdf
    HypertextPreprocessor, // .php
    MicrosoftPowerpoint, // .ppt
    MicrosoftPowerpointOpenXML, // .pptx
    RARArchive, // .rar
    RichTextFormat, // .rtf
    BourneShellScript, // .sh
    ScalableVectorGraphics, // .svg
    SmallWebFormat, // .swf
    TapeArchive, // .tar
    TaggedImage, // .tif - .tiff
    MPEGTransportStream, // .ts
    TrueTypeFont, // .ttf
    Text, // .txt
    MicrosoftVisio, // .vsd
    WaveformAudio, // .wav
    WEBMAudio, // .weba
    WEBMVideo, // .webm
    WEBMImage, // .webp
    WebOpenFontFormat, // .woff
    WepOpenFontFormat2, // .woff2,
    XHTML, // .xhtml,
    MicrosoftExcel, // .xls
    MicrosoftExcelOpenXML, // .xlsx
    XML, // .xml
    XUL, // .xul
    ZIPArchive, // .zip
    ThreeGP, // .3gp
    ThreeGP2, // .3g2
    SevenZipArchive, // .7z
    FileTypesEnd
  };

  const char *__fileTypeToString(FileTypes type);
  FileTypes __extensionToFileType(string ext);
};

#endif
