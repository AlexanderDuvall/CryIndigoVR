#include "StdAfx.h"
#include <DefaultComponents/Input/InputComponent.h>
#include <DefaultComponents/Physics/RigidBodyComponent.h>
#include <CrySchematyc/Reflection/TypeDesc.h>
#include <CrySchematyc/Utils/EnumFlags.h>
#include <CrySchematyc/Env/IEnvRegistry.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/Elements/EnvFunction.h>
#include <CrySchematyc/Env/Elements/EnvSignal.h>
#include <CrySchematyc/ResourceTypes.h>
#include <CrySchematyc/MathTypes.h>
#include <CrySchematyc/Utils/SharedString.h>
#include <CryCore/StaticInstanceList.h>
#include <CrySystem/VR/IHMDDevice.h>
#include <CrySystem/VR/IHMDManager.h>
#include <CryEntitySystem/IEntitySystem.h>
#include <iostream>
#include <math.h>
#include <CryMath/Cry_Matrix34.h>
#include <CryPhysics/physinterface.h>
#include <CryPhysics/RayCastQueue.h>
#include <vector> 
#include "VRGrabbable.h"
static void RegisterVRGrabbableComponent(Schematyc::IEnvRegistrar& registrar)
{
	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(VRGrabbable));
		// Functions	
		{
		}
	}
}
CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterVRGrabbableComponent)

/*
returns if child is an extension or not
*/
bool VRGrabbable::isExtension() {
	return VRGrabbable::isChildExtension;
}
/*
* Confirms if the main varaible is the largest of the two
* @main Variable to check
* @s Variable to compare to
* @d Variable to compare to
@return
*/
bool isLargestAxis(float main, float s, float d) {
	if ((main == s && main == d) || (main >= s && main > d)) {
		return true;
	}
	return false;
}

IEntity* VRGrabbable::getParent() {
	if (isExtension()) {
		return m_pEntity->GetParent();
	}
	else {
		return m_pEntity;
	}
}


bool VRGrabbable::isComponentActivated() {
	return isActivated;
}

void VRGrabbable::Initialize() {

}

void VRGrabbable::ProcessEvent(const SEntityEvent& event) {

}

Cry::Entity::EventFlags VRGrabbable::GetEventMask() const {
	return (ENTITY_EVENT_UPDATE);
}
//Give back Quat 
Quat VRGrabbable::getEntityAngleOffset(Ang3 handRotation, bool leftHand) {
	/// <summary>
	///Do not print or bottleneck commands. For some reason printing all attributes before processing them rids everything of their values...
	/// </summary>
	/// <param name="handRotation"></param>
	/// <param name="leftHand"></param>
	/// <returns></returns>
	if (leftHand) {
		FinalOffset = (Ang3(
			DEG2RAD(LeftXRota) + handRotation.x,
			DEG2RAD(LeftYRota) + handRotation.y,
			DEG2RAD(LeftZRota) + handRotation.z));; // variables are radians
	}
	else {
		FinalOffset = (Ang3(
			DEG2RAD(RightXRota) + handRotation.x,
			DEG2RAD(RightYRota) + handRotation.y,
			DEG2RAD(RightZRota) + handRotation.z));
	}
	return (Quat(Matrix33::CreateRotationXYZ(FinalOffset))); // do this... Maybe converts to degrees?
}
Vec3 VRGrabbable::getEntityTransformOffset(bool isRightHand) {
	if (isRightHand)
		if (RightXTrans == -1 && RightYTrans == -1 && RightZTrans == -1)
			return Vec3(LeftXTrans * -1.0f, LeftYTrans, LeftZTrans);
		else
			return Vec3(RightXTrans, RightYTrans, RightZTrans);
	return Vec3(LeftXTrans, LeftYTrans, LeftZTrans);
}