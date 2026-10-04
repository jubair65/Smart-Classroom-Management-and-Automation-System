package com.uap.cse316.smartclassroom.data.model

import com.google.firebase.database.IgnoreExtraProperties

@IgnoreExtraProperties
data class AlertItem(
    var id: String = "",
    val type: String = "UNKNOWN_PERSON",
    val message: String = "Unauthorized person detected",
    val date: String = "",
    val time: String = "",
    val timestamp: Long = 0L,
    val students: Int = 0,
    val unknownCount: Int = 0,
    val teacherPresent: Boolean = false
)
