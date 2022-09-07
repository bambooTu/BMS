/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _DIGITAL_INPUT_H /* Guard against multiple inclusion */
#define _DIGITAL_INPUT_H

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

/* TODO:  Include other files here if needed. */
#include <stdbool.h>  // Defines true

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */
/** Descriptive Constant Name

  @Summary
    Brief one-line summary of the constant.

  @Description
    Full description, explaining the purpose and usage of the constant.
    <p>
    Additional description in consecutive paragraphs separated by HTML
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

  @Remarks
    Any additional remarks
 */
#define EXAMPLE_CONSTANT 0

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

/*  A brief description of a section can be given directly below the section
    banner.
 */
typedef enum {
    DIN_1,            // Button release
    DIN_2,            // Button turn on
    DIN_3,            // Button turn off
    DIN_4,            // EMS cutoff
    DIN_MAPPING_MAX,  // max
} DIN_MAPPING_e;

typedef struct {
    bool         status;
    unsigned int debounceTime[2];
} DIN_PARAM_t;

typedef struct {
    DIN_MAPPING_e dinNum;
    DIN_PARAM_t*  dinParam;
} DIN_TASK_TABLE_t;

// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions
// *****************************************************************************
// *****************************************************************************
void DIN_ParameterInitialize(void);
void DIN_5ms_Tasks(void);
bool DIN_StateGet(DIN_MAPPING_e dinNum);

/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
