#pragma once

extern wchar_t dirpath[];


void extractfinalname(const wchar_t *src, wchar_t *dst);

void extractthis(const wchar_t *test, wchar_t *output, int *pos);

wchar_t GetDriveLetter(wchar_t *lpDevicePath);

void convertwithdrive(const wchar_t *src, int len, wchar_t *dst);

void SetupDirectory();