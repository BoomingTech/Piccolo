if (get_bool(GameObject, "MotorComponent.m_is_moving")) then
    set_float(GameObject, "MotorComponent.m_motor_res.m_jump_height", 10)
else 
    set_float(GameObject, "MotorComponent.m_motor_res.m_jump_height", 4)
end
-- invoke(GameObject, "MotorComponent.getOffStuckState")
