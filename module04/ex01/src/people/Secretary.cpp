#include "people/Secretary.hpp"
#include "forms/Form.hpp"
#include "forms/CourseFinishedForm.hpp"
#include "forms/NeedMoreClassRoomForm.hpp"
#include "forms/NeedCourseCreationForm.hpp"
#include "forms/SubscriptionToCourseForm.hpp"

Secretary::Secretary(std::string p_name) : Staff(p_name)
{
}

Form* Secretary::createForm(FormType p_formType)
{
	(void)p_formType;
	return NULL;
}

void Secretary::archiveForm()
{
}