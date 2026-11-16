#pragma once

/**
 * Matches a string against a wildcard pattern.
 *
 * The only special character in a patter is '*', which matches any number of any characters.
 *
 * Returns true if the string matches the pattern.
 */
bool stringWildcardMatch(const char* string, const char* wild);
