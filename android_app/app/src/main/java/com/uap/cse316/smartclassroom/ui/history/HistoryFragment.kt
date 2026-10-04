package com.uap.cse316.smartclassroom.ui.history

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.LinearLayoutManager
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.DatabaseError
import com.google.firebase.database.ValueEventListener
import com.uap.cse316.smartclassroom.data.model.ClassroomLive
import com.uap.cse316.smartclassroom.data.model.HistoryItem
import com.uap.cse316.smartclassroom.databinding.FragmentHistoryBinding
import com.uap.cse316.smartclassroom.utils.FirebaseManager
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class HistoryFragment : Fragment() {

    private var _binding: FragmentHistoryBinding? = null
    private val binding get() = _binding!!

    private lateinit var adapter: HistoryAdapter
    private var historyEventListener: ValueEventListener? = null
    private var liveEventListener: ValueEventListener? = null

    private var currentLiveItem: HistoryItem? = null
    private var pastHistoryItems: List<HistoryItem> = emptyList()

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentHistoryBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        adapter = HistoryAdapter()
        binding.rvHistory.layoutManager = LinearLayoutManager(requireContext())
        binding.rvHistory.adapter = adapter

        binding.swipeRefreshHistory.setOnRefreshListener {
            loadData()
        }

        binding.btnDeleteHistory.setOnClickListener {
            confirmClearHistory()
        }

        loadData()
    }

    private fun confirmClearHistory() {
        MaterialAlertDialogBuilder(requireContext())
            .setTitle("Clear Activity History?")
            .setMessage("This will permanently delete all past telemetry snapshots and history logs from Firebase.")
            .setPositiveButton("Clear All") { _, _ ->
                FirebaseManager.getHistoryReference().removeValue()
                    .addOnSuccessListener {
                        Toast.makeText(requireContext(), "History logs cleared.", Toast.LENGTH_SHORT).show()
                    }
                    .addOnFailureListener { e ->
                        Toast.makeText(requireContext(), "Failed to clear: ${e.message}", Toast.LENGTH_SHORT).show()
                    }
            }
            .setNegativeButton("Cancel", null)
            .show()
    }

    private fun loadData() {
        listenToLiveSnapshot()
        listenToHistoryRecords()
    }

    private fun listenToLiveSnapshot() {
        val liveRef = FirebaseManager.getLiveReference()
        liveEventListener?.let { liveRef.removeEventListener(it) }

        liveEventListener = object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                if (!isAdded || _binding == null) return

                val live = snapshot.getValue(ClassroomLive::class.java)
                if (live != null) {
                    val now = Date()
                    val fallbackDate = SimpleDateFormat("dd MMM yyyy", Locale.getDefault()).format(now)
                    val fallbackTime = SimpleDateFormat("hh:mm:ss a", Locale.getDefault()).format(now)

                    currentLiveItem = HistoryItem(
                        id = "CURRENT_LIVE_SNAPSHOT",
                        date = if (live.date.isNotBlank()) live.date else fallbackDate,
                        time = if (live.time.isNotBlank()) live.time else fallbackTime,
                        teacher = if (live.teacherPresent) "PRESENT" else "ABSENT",
                        students = live.studentCount,
                        unknownCount = live.unknownCount,
                        temp = live.temperature,
                        fan = if (live.fan) 1 else 0,
                        light = if (live.light) 1 else 0,
                        ac = if (live.ac) 1 else 0,
                        proj = if (live.projector) 1 else 0
                    )
                    combineAndDisplay()
                }
            }

            override fun onCancelled(error: DatabaseError) {}
        }
        liveRef.addValueEventListener(liveEventListener as ValueEventListener)
    }

    private fun listenToHistoryRecords() {
        val histQuery = FirebaseManager.getHistoryReference().limitToLast(30)
        historyEventListener?.let { histQuery.removeEventListener(it) }

        historyEventListener = object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                if (!isAdded || _binding == null) return
                binding.swipeRefreshHistory.isRefreshing = false

                val records = mutableListOf<HistoryItem>()
                for (child in snapshot.children) {
                    val key = child.key ?: ""
                    // Skip any dummy seed template entries
                    if (key.startsWith("sample", ignoreCase = true)) continue

                    val item = child.getValue(HistoryItem::class.java)
                    if (item != null) {
                        item.id = key
                        records.add(item)
                    }
                }

                // Push IDs in Firebase are chronologically ordered, reverse gives newest first
                pastHistoryItems = records.reversed()
                combineAndDisplay()
            }

            override fun onCancelled(error: DatabaseError) {
                if (!isAdded || _binding == null) return
                binding.swipeRefreshHistory.isRefreshing = false
            }
        }
        histQuery.addValueEventListener(historyEventListener as ValueEventListener)
    }

    private fun combineAndDisplay() {
        if (!isAdded || _binding == null) return

        val combinedList = mutableListOf<HistoryItem>()
        // 1. Position 0 is the real-time active snapshot if available
        currentLiveItem?.let { combinedList.add(it) }

        // 2. Position 1..N are past historical logs
        combinedList.addAll(pastHistoryItems)

        if (combinedList.isEmpty()) {
            binding.rvHistory.visibility = View.GONE
            binding.emptyHistoryView.visibility = View.VISIBLE
            binding.btnDeleteHistory.visibility = View.GONE
        } else {
            binding.rvHistory.visibility = View.VISIBLE
            binding.emptyHistoryView.visibility = View.GONE
            // Show clear button if there are past records to clear
            binding.btnDeleteHistory.visibility = if (pastHistoryItems.isNotEmpty()) View.VISIBLE else View.GONE
            adapter.setHistory(combinedList)
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        liveEventListener?.let { FirebaseManager.getLiveReference().removeEventListener(it) }
        historyEventListener?.let { FirebaseManager.getHistoryReference().limitToLast(30).removeEventListener(it) }
        _binding = null
    }

}
