package com.uap.cse316.smartclassroom.data.model

import com.google.firebase.database.IgnoreExtraProperties

@IgnoreExtraProperties
data class ClassroomLive(
    val teacherPresent: Boolean = false,
    val teacherName: String = "",
    val studentCount: Int = 0,
    val unknownCount: Int = 0,
    val temperature: Double = 26.0,
    val fan: Boolean = false,
    val light: Boolean = false,
    val ac: Boolean = false,
    val projector: Boolean = false,
    val date: String = "",
    val time: String = "",
    val lastUpdated: String = "",
    val timestamp: Long = 0L
)
