package com.uap.cse316.smartclassroom.data.model

import com.google.firebase.database.IgnoreExtraProperties

@IgnoreExtraProperties
data class HistoryItem(
    var id: String = "",
    val date: String = "",
    val time: String = "",
    val timestamp: Long = 0L,
    val teacher: String = "ABSENT",
    val students: Int = 0,
    val unknownCount: Int = 0,
    val temp: Double = 26.0,
    val fan: Int = 0,
    val light: Int = 0,
    val ac: Int = 0,
    val proj: Int = 0
)
