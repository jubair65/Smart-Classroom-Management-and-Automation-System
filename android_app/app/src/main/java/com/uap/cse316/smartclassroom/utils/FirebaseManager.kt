package com.uap.cse316.smartclassroom.utils

import com.google.firebase.database.DatabaseReference
import com.google.firebase.database.FirebaseDatabase

object FirebaseManager {
    // Explicit Database URL matching the user's Firebase region
    const val DATABASE_URL = "https://smart-classroom-team06-default-rtdb.asia-southeast1.firebasedatabase.app"

    private val database: FirebaseDatabase by lazy {
        FirebaseDatabase.getInstance(DATABASE_URL).apply {
            try {
                setPersistenceEnabled(true)
            } catch (e: Exception) {
                // Persistence already configured
            }
        }
    }

    fun getLiveReference(): DatabaseReference {
        return database.getReference("classroom/live").apply {
            keepSynced(true)
        }
    }

    fun getAlertsReference(): DatabaseReference {
        return database.getReference("classroom/alerts").apply {
            keepSynced(true)
        }
    }

    fun getHistoryReference(): DatabaseReference {
        return database.getReference("classroom/history").apply {
            keepSynced(true)
        }
    }
}

