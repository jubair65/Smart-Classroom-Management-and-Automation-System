package com.uap.cse316.smartclassroom.ui.dashboard

import android.content.Context
import android.content.Intent
import android.graphics.Color
import android.media.AudioManager
import android.media.RingtoneManager
import android.media.ToneGenerator
import android.os.Build
import android.os.Bundle
import android.os.VibrationEffect
import android.os.Vibrator
import android.os.VibratorManager
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import android.widget.Toast
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import com.google.android.material.bottomnavigation.BottomNavigationView
import com.google.android.material.card.MaterialCardView
import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.DatabaseError
import com.google.firebase.database.ValueEventListener
import com.uap.cse316.smartclassroom.R
import com.uap.cse316.smartclassroom.data.model.ClassroomLive
import com.uap.cse316.smartclassroom.databinding.FragmentDashboardBinding
import com.uap.cse316.smartclassroom.ui.students.StudentsActivity
import com.uap.cse316.smartclassroom.utils.FirebaseManager
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class DashboardFragment : Fragment() {

    private var _binding: FragmentDashboardBinding? = null
    private val binding get() = _binding!!

    private var liveEventListener: ValueEventListener? = null
    private var lastRecordedUnknownCount: Int = -1

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentDashboardBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        // Set real local phone date & time initially
        updateLocalHeaderTime()

        binding.swipeRefresh.setOnRefreshListener {
            updateLocalHeaderTime()
            binding.swipeRefresh.isRefreshing = false
        }

        // Clicking on Students card opens the Students list window
        binding.cardStudents.setOnClickListener {
            val intent = Intent(requireContext(), StudentsActivity::class.java)
            startActivity(intent)
        }

        // Clicking on Unknown card navigates to Alerts tab
        binding.cardUnknownAlerts.setOnClickListener {
            val bottomNav = requireActivity().findViewById<BottomNavigationView>(R.id.bottomNavigation)
            bottomNav?.selectedItemId = R.id.nav_alerts
        }

        listenToLiveClassroom()
    }

    private fun updateLocalHeaderTime() {
        val dateFmt = SimpleDateFormat("EEEE, dd MMM yyyy", Locale.getDefault())
        val timeFmt = SimpleDateFormat("hh:mm:ss a", Locale.getDefault())
        val now = Date()

        _binding?.let {
            it.tvLiveDate.text = "📅 " + dateFmt.format(now)
            it.tvLiveClock.text = timeFmt.format(now)
        }
    }

    private fun listenToLiveClassroom() {
        val liveRef = FirebaseManager.getLiveReference()

        liveEventListener = object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                if (!isAdded || _binding == null) return

                val data = snapshot.getValue(ClassroomLive::class.java) ?: ClassroomLive()
                updateUI(data)

                // Sound BEEP and VIBRATION on new unknown intrusion
                if (lastRecordedUnknownCount != -1 && data.unknownCount > lastRecordedUnknownCount) {
                    triggerIntrusionAlarm()
                }
                lastRecordedUnknownCount = data.unknownCount
            }

            override fun onCancelled(error: DatabaseError) {
                if (!isAdded || _binding == null) return
                binding.tvLastSync.text = "Error connecting to Firebase: ${error.message}"
            }
        }

        liveRef.addValueEventListener(liveEventListener as ValueEventListener)
    }

    private fun triggerIntrusionAlarm() {
        val ctx = context ?: return
        try {
            // 1. Play high-frequency warning tone through phone speaker
            val toneGen = ToneGenerator(AudioManager.STREAM_ALARM, 100)
            toneGen.startTone(ToneGenerator.TONE_CDMA_EMERGENCY_RINGBACK, 500)

            // 2. Play notification sound
            val notificationUri = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION)
            val ringtone = RingtoneManager.getRingtone(ctx, notificationUri)
            ringtone?.play()

            // 3. Vibrate the phone
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                val vibratorManager = ctx.getSystemService(Context.VIBRATOR_MANAGER_SERVICE) as? VibratorManager
                vibratorManager?.defaultVibrator?.vibrate(VibrationEffect.createOneShot(500, VibrationEffect.DEFAULT_AMPLITUDE))
            } else {
                @Suppress("DEPRECATION")
                val vibrator = ctx.getSystemService(Context.VIBRATOR_SERVICE) as? Vibrator
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                    vibrator?.vibrate(VibrationEffect.createOneShot(500, VibrationEffect.DEFAULT_AMPLITUDE))
                } else {
                    @Suppress("DEPRECATION")
                    vibrator?.vibrate(500)
                }
            }

            Toast.makeText(ctx, "🚨 SECURITY ALERT: Unauthorized Person Detected!", Toast.LENGTH_LONG).show()
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    private fun updateUI(data: ClassroomLive) {
        val context = requireContext()

        // 1. Teacher Status
        if (data.teacherPresent) {
            binding.tvTeacherName.text = if (data.teacherName.isNotBlank()) "${data.teacherName} (Lecturer)" else "Sayma (Lecturer)"
            binding.tvTeacherBadge.text = getString(R.string.status_present)
            binding.tvTeacherBadge.setTextColor(ContextCompat.getColor(context, R.color.status_green))
            binding.tvTeacherBadge.setBackgroundColor(ContextCompat.getColor(context, R.color.status_green_light))
        } else {
            binding.tvTeacherName.text = "No teacher in class"
            binding.tvTeacherBadge.text = getString(R.string.status_absent)
            binding.tvTeacherBadge.setTextColor(ContextCompat.getColor(context, R.color.status_red))
            binding.tvTeacherBadge.setBackgroundColor(ContextCompat.getColor(context, R.color.status_red_light))
        }

        // 2. Student Count
        binding.tvStudentCount.text = data.studentCount.toString()

        // 3. Unknown Intruder Count
        binding.tvUnknownCount.text = data.unknownCount.toString()
        if (data.unknownCount > 0) {
            binding.tvUnknownStatus.text = "⚠️ Intrusions flagged (Tap to view)"
            binding.tvUnknownStatus.setTextColor(ContextCompat.getColor(context, R.color.status_red))
            binding.cardUnknownAlerts.strokeWidth = 2
            binding.cardUnknownAlerts.strokeColor = ContextCompat.getColor(context, R.color.status_red)
        } else {
            binding.tvUnknownStatus.text = "Entrance is secure"
            binding.tvUnknownStatus.setTextColor(ContextCompat.getColor(context, R.color.text_muted))
            binding.cardUnknownAlerts.strokeWidth = 1
            binding.cardUnknownAlerts.strokeColor = ContextCompat.getColor(context, R.color.divider)
        }

        // 4. Temperature
        binding.tvTemperature.text = String.format("%.1f°C", data.temperature)
        when {
            data.temperature >= 30.0 -> {
                binding.tvTempStatus.text = "High Heat (Fan & AC ON)"
                binding.tvTempBadge.text = "High Temp"
                binding.tvTempBadge.setTextColor(ContextCompat.getColor(context, R.color.status_red))
                binding.tvTempBadge.setBackgroundColor(ContextCompat.getColor(context, R.color.status_red_light))
            }
            data.temperature >= 27.0 -> {
                binding.tvTempStatus.text = "Warm (Fan Auto ON)"
                binding.tvTempBadge.text = "Warm"
                binding.tvTempBadge.setTextColor(ContextCompat.getColor(context, R.color.status_orange))
                binding.tvTempBadge.setBackgroundColor(ContextCompat.getColor(context, R.color.status_orange_light))
            }
            data.temperature <= 25.5 -> {
                binding.tvTempStatus.text = "Cool / Normal Room"
                binding.tvTempBadge.text = "Comfortable"
                binding.tvTempBadge.setTextColor(ContextCompat.getColor(context, R.color.status_green))
                binding.tvTempBadge.setBackgroundColor(ContextCompat.getColor(context, R.color.status_green_light))
            }
            else -> {
                binding.tvTempStatus.text = "Ambient Classroom Temp"
                binding.tvTempBadge.text = "Normal"
                binding.tvTempBadge.setTextColor(ContextCompat.getColor(context, R.color.primary))
                binding.tvTempBadge.setBackgroundColor(ContextCompat.getColor(context, R.color.status_blue_light))
            }
        }

        // 5. Appliances
        updateApplianceCard(binding.cardFan, binding.tvFanStatus, data.fan, R.color.fan_active, "🌀 Fan")
        updateApplianceCard(binding.cardLight, binding.tvLightStatus, data.light, R.color.light_active, "💡 Light")
        updateApplianceCard(binding.cardAc, binding.tvAcStatus, data.ac, R.color.ac_active, "❄️ AC")
        updateApplianceCard(binding.cardProj, binding.tvProjStatus, data.projector, R.color.projector_active, "📽️ Proj")

        // 6. Header Clock & Footer Last Sync with Local Time
        val timeFmt = SimpleDateFormat("hh:mm:ss a", Locale.getDefault())
        val dateFmt = SimpleDateFormat("EEEE, dd MMM yyyy", Locale.getDefault())
        val now = Date()

        val displayTime = if (data.time.isNotBlank()) data.time else timeFmt.format(now)
        val displayDate = if (data.date.isNotBlank()) data.date else dateFmt.format(now)

        binding.tvLiveDate.text = "📅 $displayDate"
        binding.tvLiveClock.text = displayTime
        binding.tvLastSync.text = "Firebase Live • Synced at $displayDate $displayTime"
    }

    private fun updateApplianceCard(
        card: MaterialCardView,
        statusText: TextView,
        isOn: Boolean,
        activeColorRes: Int,
        label: String
    ) {
        val context = requireContext()
        if (isOn) {
            statusText.text = getString(R.string.status_on)
            statusText.setTextColor(ContextCompat.getColor(context, activeColorRes))
            card.strokeWidth = 2
            card.strokeColor = ContextCompat.getColor(context, activeColorRes)
        } else {
            statusText.text = getString(R.string.status_off)
            statusText.setTextColor(ContextCompat.getColor(context, R.color.appliance_inactive))
            card.strokeWidth = 1
            card.strokeColor = ContextCompat.getColor(context, R.color.divider)
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        liveEventListener?.let {
            FirebaseManager.getLiveReference().removeEventListener(it)
        }
        _binding = null
    }
}
